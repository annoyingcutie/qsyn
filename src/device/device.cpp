/****************************************************************************
  PackageName  [ device ]
  Synopsis     [ Define class Device, Topology, and Operation functions ]
  Author       [ Design Verification Lab ]
  Copyright    [ Copyright(c) 2023 DVLab, GIEE, NTU, Taiwan ]
****************************************************************************/

#include "device/device.hpp"

#include <fmt/ranges.h>
#include <fmt/std.h>
#include <spdlog/spdlog.h>

#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <gsl/narrow>
#include <limits>
#include <ranges>
#include <string>
#include <utility>

#include "qcir/gate_type.hpp"
#include "qcir/qcir_gate.hpp"
#include "qsyn/qsyn_type.hpp"
#include "util/util.hpp"

using namespace qsyn::qcir;

template <>
struct fmt::formatter<qsyn::device::PhysicalQubit> {
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(qsyn::device::PhysicalQubit const& q, FormatContext& ctx) {
        return fmt::format_to(ctx.out(), "Q{:>2}, logical: {:>2}, lock until {}", q.get_id(), q.get_logical_qubit(), q.get_occupied_time());
    }
};

namespace qsyn::device {

// SECTION - Struct Info Member Functions

/**
 * @brief Print overloading
 *
 * @param os
 * @param info
 * @return ostream&
 */
std::ostream& operator<<(std::ostream& os, DeviceInfo const& info) {
    return os << fmt::format("{}", info);
}

// SECTION - Class Topology Member Functions

/**
 * @brief Get the information of a single adjacency pair
 *
 * @param a Id of first qubit
 * @param b Id of second qubit
 * @return Info&
 */
DeviceInfo const& Topology::get_adjacency_pair_info(size_t a, size_t b) {
    if (a > b) std::swap(a, b);
    return _adjacency_info[std::make_pair(a, b)];
}

/**
 * @brief Get the information of a qubit
 *
 * @param a
 * @return const Info&
 */
DeviceInfo const& Topology::get_qubit_info(size_t a) {
    return _qubit_info[a];
}

/**
 * @brief Add adjacency information of (a,b)
 *
 * @param a Id of first qubit
 * @param b Id of second qubit
 * @param info Information of this pair
 */
void Topology::add_adjacency_info(size_t a, size_t b, DeviceInfo info) {
    if (a > b) std::swap(a, b);
    _adjacency_info[std::make_pair(a, b)] = info;
}

/**
 * @brief Add qubit information
 *
 * @param a
 * @param info
 */
void Topology::add_qubit_info(size_t a, DeviceInfo info) {
    _qubit_info[a] = info;
}

/**
 * @brief Print information of the edge (a,b)
 *
 * @param a Index of first qubit
 * @param b Index of second qubit
 */
void Topology::print_single_edge(size_t a, size_t b) const {
    auto query = (a < b) ? std::make_pair(a, b) : std::make_pair(b, a);
    if (_adjacency_info.contains(query)) {
        fmt::println("({:>3}, {:>3})    Delay: {:>8.3f}    Error: {:>8.5f}", a, b, _adjacency_info.at(query)._time, _adjacency_info.at(query)._error);
    } else {
        fmt::println("No connection between {:>3} and {:>3}.", a, b);
    }
}

// SECTION - Class PhysicalQubit Member Functions

/**
 * @brief Operator overloading
 *
 * @param os
 * @param q
 * @return ostream&
 */
std::ostream& operator<<(std::ostream& os, PhysicalQubit const& q) {
    return os << fmt::format("{}", q);
}

/**
 * @brief Mark qubit
 *
 * @param source false: from 0, true: from 1
 * @param pred predecessor
 */
void PhysicalQubit::mark(bool source, QubitIdType pred) {
    _marked = true;
    _source = source;
    _pred   = pred;
}

/**
 * @brief Take the route
 *
 * @param cost
 * @param swapTime
 */
void PhysicalQubit::take_route(size_t cost, size_t swap_time) {
    _cost      = cost;
    _swap_time = swap_time;
    _taken     = true;
}

/**
 * @brief Reset qubit
 *
 */
void PhysicalQubit::reset() {
    _marked = false;
    _taken  = false;
    _cost   = _occupied_time;
}

// SECTION - Class Device Member Functions

/**
 * @brief Get next swap cost
 *
 * @param source
 * @param target
 * @return tuple<size_t, size_t> (index of next qubit, cost)
 */
std::tuple<QubitIdType, QubitIdType> Device::get_next_swap_cost(QubitIdType source, QubitIdType target) {
    auto next_idx           = _predecessor[source][target];
    PhysicalQubit& q_source = get_physical_qubit(source);
    PhysicalQubit& q_next   = get_physical_qubit(next_idx);
    size_t cost             = std::max(q_source.get_occupied_time(), q_next.get_occupied_time());

    assert(q_source.is_adjacency(q_next));
    return {next_idx, cost};
}

/**
 * @brief Get physical qubit id by logical id
 *
 * @param id logical
 * @return size_t
 */
QubitIdType Device::get_physical_by_logical(QubitIdType id) {
    for (auto& [_, phy] : _qubit_list) {
        if (phy.get_logical_qubit() == id) {
            return phy.get_id();
        }
    }
    return max_qubit_id;
}

/**
 * @brief Add adjacency pair (a,b)
 *
 * @param a Id of first qubit
 * @param b Id of second qubit
 */
void Device::add_adjacency(QubitIdType a, QubitIdType b) {
    if (a > b) std::swap(a, b);
    if (!qubit_id_exists(a)) {
        PhysicalQubit temp = PhysicalQubit(a);
        add_physical_qubit(temp);
    }
    if (!qubit_id_exists(b)) {
        PhysicalQubit temp = PhysicalQubit(b);
        add_physical_qubit(temp);
    }
    _qubit_list[a].add_adjacency(_qubit_list[b].get_id());
    _qubit_list[b].add_adjacency(_qubit_list[a].get_id());
    constexpr DeviceInfo default_info = {._time = 0.0, ._error = 0.0};
    _topology->add_adjacency_info(a, b, default_info);
}

/**
 * @brief Apply gate to device
 *
 * @param op
 */
void Device::apply_gate(Operation const& op) {
    auto qubits = op.get_qubits();
    auto& q0    = get_physical_qubit(std::get<0>(qubits));
    auto& q1    = get_physical_qubit(std::get<1>(qubits));
    auto t      = op.get_time_begin();

    if (op.is_swap()) {
        auto temp = q0.get_logical_qubit();
        q0.set_logical_qubit(q1.get_logical_qubit());
        q1.set_logical_qubit(temp);
        q0.set_occupied_time(t + SWAP_DELAY);
        q1.set_occupied_time(t + SWAP_DELAY);
    } else if (op.is_cx()) {
        q0.set_occupied_time(t + DOUBLE_DELAY);
        q1.set_occupied_time(t + DOUBLE_DELAY);
    } else if (op.is_cz()) {
        q0.set_occupied_time(t + DOUBLE_DELAY);
        q1.set_occupied_time(t + DOUBLE_DELAY);
    } else {
        DVLAB_ASSERT(false, "Unknown gate type at apply_gate()!!");
    }
}

/**
 * @brief Apply swap, only used in checker
 *
 * @param op
 */
void Device::apply_swap_check(QubitIdType qid0, QubitIdType qid1) {
    auto& q0  = get_physical_qubit(qid0);
    auto& q1  = get_physical_qubit(qid1);
    auto temp = q0.get_logical_qubit();
    q0.set_logical_qubit(q1.get_logical_qubit());
    q1.set_logical_qubit(temp);
    size_t t = std::max(q0.get_occupied_time(), q1.get_occupied_time());
    q0.set_occupied_time(t + DOUBLE_DELAY);
    q1.set_occupied_time(t + DOUBLE_DELAY);
}

/**
 * @brief Apply single-qubit gate
 *
 * @param physicalId
 */
void Device::apply_single_qubit_gate(QubitIdType physical_id) {
    size_t start_time = _qubit_list[physical_id].get_occupied_time();
    _qubit_list[physical_id].set_occupied_time(start_time + SINGLE_DELAY);
    _qubit_list[physical_id].reset();
}

/**
 * @brief get mapping of physical qubit
 *
 * @return vector<size_t> (index of physical qubit)
 */
std::vector<std::optional<size_t>> Device::mapping() const {
    std::vector<std::optional<size_t>> ret;
    ret.resize(_qubit_list.size());
    for (auto const& [id, qubit] : _qubit_list) {
        ret[id] = qubit.get_logical_qubit();
    }
    return ret;
}

/**
 * @brief Place logical qubit
 *
 * @param assign
 */
void Device::place(std::vector<QubitIdType> const& assignment) {
    for (size_t i = 0; i < assignment.size(); ++i) {
        assert(_qubit_list[assignment[i]].get_logical_qubit() == std::nullopt);
        _qubit_list[assignment[i]].set_logical_qubit(i);
    }
}

/**
 * @brief Calculate Shortest Path
 *
 */
void Device::calculate_path() {
    _predecessor.clear();
    _distance.clear();
    _adjacency_matrix.clear();
    _adjacency_matrix.resize(get_num_qubits());
    for (size_t i = 0; i < get_num_qubits(); i++) {
        _adjacency_matrix[i].resize(get_num_qubits(), _max_dist);
        for (size_t j = 0; j < get_num_qubits(); j++) {
            if (i == j)
                _adjacency_matrix[i][j] = 0;
        }
    }
    floyd_warshall();
    _adjacency_matrix.clear();
}

/**
 * @brief Init data for Floyd-Warshall Algorithm
 *
 */
void Device::_initialize_floyd_warshall() {
    _distance.resize(get_num_qubits());
    _predecessor.resize(get_num_qubits());

    for (size_t i = 0; i < get_num_qubits(); i++) {
        _distance[i].resize(get_num_qubits());
        _predecessor[i].resize(get_num_qubits(), max_qubit_id);
        for (size_t j = 0; j < get_num_qubits(); j++) {
            _distance[i][j] = _adjacency_matrix[i][j];
            if (_distance[i][j] != 0 && _distance[i][j] != _max_dist) {
                _predecessor[i][j] = _qubit_list[gsl::narrow<QubitIdType>(i)].get_id();
            }
        }
    }

    spdlog::debug("Predecessor Matrix:");
    for (auto& row : _predecessor) {
        spdlog::debug("{:5}", fmt::join(row | std::views::transform([](QubitIdType j) { return (j == max_qubit_id) ? std::string{"/"} : std::to_string(j); }), ""));
    }
    spdlog::debug("Distance Matrix:");
    for (auto& row : _distance) {
        spdlog::debug("{:5}", fmt::join(row | std::views::transform([this](int j) { return (j == _max_dist) ? std::string{"X"} : std::to_string(j); }), ""));
    }
}

/**
 * @brief Set weight of edge used in Floyd-Warshall
 *
 * @param type
 */
void Device::_set_weight() {
    assert(_adjacency_matrix.size() == _num_qubit);
    for (size_t i = 0; i < _num_qubit; i++) {
        for (auto const& adj : _qubit_list[gsl::narrow<QubitIdType>(i)].get_adjacencies()) {
            _adjacency_matrix[i][adj] = 1;
        }
    }
}

/**
 * @brief Floyd-Warshall Algorithm. Solve All Pairs Shortest Path (APSP)
 *
 */
void Device::floyd_warshall() {
    _set_weight();
    _initialize_floyd_warshall();
    for (size_t k = 0; k < _num_qubit; k++) {
        spdlog::debug("Including vertex({}):", k);
        for (size_t i = 0; i < _num_qubit; i++) {
            for (size_t j = 0; j < _num_qubit; j++) {
                if ((_distance[i][j] > _distance[i][k] + _distance[k][j]) && (_distance[i][k] != _max_dist)) {
                    _distance[i][j]    = _distance[i][k] + _distance[k][j];
                    _predecessor[i][j] = _predecessor[k][j];
                }
            }
        }

        spdlog::debug("Predecessor Matrix:");
        for (auto& row : _predecessor) {
            spdlog::debug("{:5}", fmt::join(
                                      row | std::views::transform([](auto j) { return (j == max_qubit_id) ? std::string{"/"} : std::to_string(j); }), ""));
        }
        spdlog::debug("Distance Matrix:");
        for (auto& row : _distance) {
            spdlog::debug("{:5}", fmt::join(
                                      row | std::views::transform([this](int j) { return (j == _max_dist) ? std::string{"X"} : std::to_string(j); }), ""));
        }
    }
}

/**
 * @brief Get shortest path from `s` to `t`
 *
 * @param s start
 * @param t terminate
 * @return vector<PhyQubit>&
 */
std::vector<PhysicalQubit> Device::get_path(QubitIdType src, QubitIdType dest) const {
    std::vector<PhysicalQubit> path;
    path.emplace_back(_qubit_list.at(src));
    if (src == dest) return path;
    auto new_pred = _predecessor[dest][src];
    path.emplace_back(new_pred);
    while (true) {
        new_pred = _predecessor[dest][new_pred];
        if (new_pred == max_qubit_id) break;
        path.emplace_back(_qubit_list.at(new_pred));
    }
    return path;
}

/**
 * @brief Read Device
 *
 * @param filename
 * @return true
 * @return false
 */
bool Device::read_device(std::string const& filename) {
    std::ifstream topo_file(filename);
    if (!topo_file.is_open()) {
        spdlog::error("Cannot open the file \"{}\"!!", filename);
        return false;
    }
    std::string str = "", token = "", data = "";

    // NOTE - Device name
    while (str == "") {
        std::getline(topo_file, str);
        str = dvlab::str::trim_spaces(dvlab::str::trim_comments(str));
    }
    size_t token_end = dvlab::str::str_get_token(str, token, 0, ": ");
    data             = str.substr(token_end + 1);

    _topology->set_name(dvlab::str::trim_spaces(data));

    // NOTE - Qubit num
    str = "", token = "", data = "";
    unsigned qbn = 0;
    while (str == "") {
        std::getline(topo_file, str);
        str = dvlab::str::trim_spaces(dvlab::str::trim_comments(str));
    }
    token_end = dvlab::str::str_get_token(str, token, 0, ": ");
    data      = str.substr(token_end + 1);
    data      = dvlab::str::trim_spaces(data);
    if (!dvlab::str::str_to_u(data, qbn)) {
        spdlog::error("The number of qubit is not a positive integer!!");
        return false;
    }
    _num_qubit = size_t(qbn);

    // NOTE - Gate set
    str = "", token = "", data = "";
    while (str == "") {
        std::getline(topo_file, str);
        str = dvlab::str::trim_spaces(dvlab::str::trim_comments(str));
    }
    if (!_parse_gate_set(str)) return false;

    // NOTE - Coupling map
    str = "", token = "", data = "";
    while (str == "") {
        std::getline(topo_file, str);
        str = dvlab::str::trim_spaces(dvlab::str::trim_comments(str));
    }

    token_end = dvlab::str::str_get_token(str, token, 0, ": ");
    data      = str.substr(token_end + 1);
    data      = dvlab::str::trim_spaces(data);
    data      = dvlab::str::remove_brackets(data, '[', ']');
    std::vector<std::vector<float>> cx_err, cx_delay;
    std::vector<std::vector<size_t>> adj_list;
    std::vector<float> sg_err, sg_delay;
    if (!_parse_size_t_pairs(data, adj_list))
        return false;

    // NOTE - Parse Information
    if (!_parse_info(topo_file, cx_err, cx_delay, sg_err, sg_delay)) return false;

    // NOTE - Finish parsing, store the topology
    for (size_t i = 0; i < adj_list.size(); i++) {
        for (size_t j = 0; j < adj_list[i].size(); j++) {
            if (adj_list[i][j] > i) {
                add_adjacency(gsl::narrow<QubitIdType>(i), gsl::narrow<QubitIdType>(adj_list[i][j]));
                _topology->add_adjacency_info(i, adj_list[i][j], {._time = cx_delay[i][j], ._error = cx_err[i][j]});
            }
        }
    }

    assert(sg_err.size() == sg_delay.size());
    for (size_t i = 0; i < sg_err.size(); i++) {
        _topology->add_qubit_info(i, {._time = sg_delay[i], ._error = sg_err[i]});
    }

    calculate_path();
    return true;
}

/**
 * @brief Parse gate set
 *
 * @param str
 * @return true
 * @return false
 */
bool Device::_parse_gate_set(std::string const& gate_set_str) {
    std::string token = "", data = "", gt;
    size_t token_end = dvlab::str::str_get_token(gate_set_str, token, 0, ": ");
    data             = gate_set_str.substr(token_end + 1);
    data             = dvlab::str::trim_spaces(data);
    data             = dvlab::str::remove_brackets(data, '{', '}');
    size_t m         = 0;
    while (m < data.size()) {
        m              = dvlab::str::str_get_token(data, gt, m, ',');
        gt             = dvlab::str::trim_spaces(gt);
        gt             = dvlab::str::tolower_string(gt);
        auto gate_type = str_to_gate_type(gt);
        if (!gate_type.has_value()) {
            spdlog::error("unsupported gate type \"{}\"!!", gt);
            return false;
        }
        _topology->add_gate_type(gate_type.value());
    }
    return true;
}

/**
 * @brief Parse device information including SGERROR, SGTIME, CNOTERROR, and CNOTTIME
 *
 * @param f
 * @param cxErr
 * @param cxDelay
 * @param sgErr
 * @param sgDelay
 * @return true
 * @return false
 */
bool Device::_parse_info(std::ifstream& f, std::vector<std::vector<float>>& cx_error, std::vector<std::vector<float>>& cx_delay, std::vector<float>& single_error, std::vector<float>& single_delay) {
    std::string str = "", token = "", data = "";
    while (true) {
        while (str == "") {
            if (f.eof()) break;
            std::getline(f, str);
            str = dvlab::str::trim_spaces(dvlab::str::trim_comments(str));
        }
        size_t token_end = dvlab::str::str_get_token(str, token, 0, ": ");
        data             = str.substr(token_end + 1);

        data = dvlab::str::trim_spaces(data);
        if (token == "SGERROR") {
            if (!_parse_singles(data, single_error)) return false;
        } else if (token == "SGTIME") {
            if (!_parse_singles(data, single_delay)) return false;
        } else if (token == "CNOTERROR") {
            if (!_parse_float_pairs(data, cx_error)) return false;
        } else if (token == "CNOTTIME") {
            if (!_parse_float_pairs(data, cx_delay)) return false;
        }
        if (f.eof()) {
            break;
        }
        std::getline(f, str);
        str = dvlab::str::trim_spaces(dvlab::str::trim_comments(str));
    }

    return true;
}

/**
 * @brief Parse device qubits information
 *
 * @param data
 * @param container
 * @return true
 * @return false
 */
bool Device::_parse_singles(std::string const& data, std::vector<float>& container) {
    std::string str, num;
    float fl = 0.;
    size_t m = 0;

    str = dvlab::str::remove_brackets(data, '[', ']');

    std::vector<float> single_fl;
    while (m < str.size()) {
        m   = dvlab::str::str_get_token(str, num, m, ',');
        num = dvlab::str::trim_spaces(num);
        if (!dvlab::str::str_to_f(num, fl)) {
            spdlog::error("The number `{}` is not a float!!", num);
            return false;
        }
        container.emplace_back(fl);
    }
    return true;
}

/**
 * @brief Parse device edges information with type is float
 *
 * @param data
 * @param container
 * @return true
 * @return false
 */
bool Device::_parse_float_pairs(std::string const& data, std::vector<std::vector<float>>& containers) {
    std::string str, num;
    float fl = 0.;
    size_t n = 0, m = 0;
    while (n < data.size()) {
        n   = dvlab::str::str_get_token(data, str, n, '[');
        str = str.substr(0, str.find_first_of(']'));
        m   = 0;
        std::vector<float> single_fl;
        while (m < str.size()) {
            m   = dvlab::str::str_get_token(str, num, m, ',');
            num = dvlab::str::trim_spaces(num);
            if (!dvlab::str::str_to_f(num, fl)) {
                spdlog::error("The number `{}` is not a float!!", num);
                return false;
            }
            single_fl.emplace_back(fl);
        }
        containers.emplace_back(single_fl);
    }
    return true;
}

/**
 * @brief Parse device edges information with type is size_t
 *
 * @param data
 * @param container
 * @return true
 * @return false
 */
bool Device::_parse_size_t_pairs(std::string const& data, std::vector<std::vector<size_t>>& containers) {
    std::string str, num;
    unsigned qbn = 0;
    size_t n = 0, m = 0;
    while (n < data.size()) {
        n   = dvlab::str::str_get_token(data, str, n, '[');
        str = str.substr(0, str.find_first_of(']'));
        m   = 0;
        std::vector<size_t> single;
        while (m < str.size()) {
            m   = dvlab::str::str_get_token(str, num, m, ',');
            num = dvlab::str::trim_spaces(num);
            if (!dvlab::str::str_to_u(num, qbn) || qbn >= _num_qubit) {
                spdlog::error("The number of qubit `{}` is not a positive integer or not in the legal range!!", num);
                return false;
            }
            single.emplace_back(size_t(qbn));
        }
        containers.emplace_back(single);
    }
    return true;
}

/**
 * @brief Print physical qubits and their adjacencies
 *
 * @param cand a vector of qubits to be printed
 */
void Device::print_qubits(std::vector<size_t> candidates) const {
    for (auto& c : candidates) {
        if (c >= _num_qubit) {
            spdlog::error("Error: the maximum qubit id is {}!!", _num_qubit - 1);
            return;
        }
    }
    fmt::println("");
    std::vector<PhysicalQubit> qubits;
    qubits.resize(_num_qubit);
    for (auto const& [idx, info] : _qubit_list) {
        qubits[idx] = info;
    }
    if (candidates.empty()) {
        for (size_t i = 0; i < qubits.size(); i++) {
            fmt::println("ID: {:>3}    {}Adjs: {:>3}", i, _topology->get_qubit_info(i), fmt::join(qubits[i].get_adjacencies(), " "));
        }
        fmt::println("Total #Qubits: {}", _num_qubit);
    } else {
        sort(candidates.begin(), candidates.end());
        for (auto& p : candidates) {
            fmt::println("ID: {:>3}    {}Adjs: {:>3}", p, _topology->get_qubit_info(p), fmt::join(qubits[p].get_adjacencies(), " "));
        }
    }
}

/**
 * @brief Print device edge
 *
 * @param cand Empty: print all. Single element [a]: print edges connecting to a. Two elements [a,b]: print edge (a,b).
 */
void Device::print_edges(std::vector<size_t> candidates) const {
    for (auto& c : candidates) {
        if (c >= _num_qubit) {
            spdlog::error("the maximum qubit id is {}!!", _num_qubit - 1);
            return;
        }
    }
    fmt::println("");
    std::vector<PhysicalQubit> qubits;
    qubits.resize(_num_qubit);
    for (auto const& [idx, info] : _qubit_list) {
        qubits[idx] = info;
    }
    if (candidates.size() == 0) {
        size_t cnt = 0;
        for (size_t i = 0; i < _num_qubit; i++) {
            for (auto& q : qubits[i].get_adjacencies()) {
                if (std::cmp_less(i, q)) {
                    cnt++;
                    _topology->print_single_edge(i, q);
                }
            }
        }
        assert(cnt == _topology->get_num_adjacencies());
        fmt::println("Total #Edges: {}", cnt);
    } else if (candidates.size() == 1) {
        for (auto& q : qubits[candidates[0]].get_adjacencies()) {
            _topology->print_single_edge(candidates[0], q);
        }
        fmt::println("Total #Edges: {}", qubits[candidates[0]].get_adjacencies().size());
    } else if (candidates.size() == 2) {
        _topology->print_single_edge(candidates[0], candidates[1]);
    }
}

/**
 * @brief Print information of Topology
 *
 */
void Device::print_topology() const {
    fmt::println("Topology: {} ({} qubits, {} edges)", get_name(), _qubit_list.size(), _topology->get_num_adjacencies());
    fmt::println("Gate Set: {}", fmt::join(_topology->get_gate_set() | std::views::transform([](GateType gtype) { return dvlab::str::toupper_string(gate_type_to_str(gtype)); }), ", "));
}

/**
 * @brief Print Predecessor
 *
 */
void Device::print_predecessor() const {
    fmt::println("Predecessor Matrix:");
    for (auto& row : _predecessor) {
        fmt::println("{:5}", fmt::join(row | std::views::transform([](auto pred) { return (pred == max_qubit_id) ? "/" : std::to_string(pred); }), ""));
    }
}

/**
 * @brief Print Distance
 *
 */
void Device::print_distance() const {
    fmt::println("Distance Matrix:");
    for (auto& row : _distance) {
        fmt::println("{:5}", fmt::join(row | std::views::transform([this](int dist) { return (dist == _max_dist) ? "X" : std::to_string(dist); }), ""));
    }
}

/**
 * @brief Print shortest path from `s` to `t`
 *
 * @param s start
 * @param t terminate
 */
void Device::print_path(QubitIdType src, QubitIdType dest) const {
    fmt::println("");
    for (auto& c : {src, dest}) {
        if (std::cmp_greater_equal(c, _num_qubit)) {
            spdlog::error("the maximum qubit id is {}!!", _num_qubit - 1);
            return;
        }
    }
    std::vector<PhysicalQubit> const& path = get_path(src, dest);
    if (path.front().get_id() != src && path.back().get_id() != dest)
        fmt::println("No path between {} and {}", src, dest);
    else {
        fmt::println("Path from {} to {}:", src, dest);
        size_t cnt = 0;
        for (auto& v : path) {
            constexpr size_t num_cols = 10;
            fmt::print("{:4} ", v.get_id());
            if (++cnt % num_cols == 0) fmt::println("");
        }
    }
}

/**
 * @brief Print Mapping (Physical : Logical)
 *
 */
void Device::print_mapping() {
    fmt::println("----------Mapping---------");
    for (size_t i = 0; i < _num_qubit; i++) {
        fmt::println("{:<5} : {}", i, _qubit_list[gsl::narrow<QubitIdType>(i)].get_logical_qubit());
    }
}

/**
 * @brief Print device status
 *
 */
void Device::print_status() const {
    fmt::println("Device Status:");
    std::vector<PhysicalQubit> qubits;
    qubits.resize(_num_qubit);
    for (auto const& [idx, info] : _qubit_list) {
        qubits[idx] = info;
    }
    for (size_t i = 0; i < qubits.size(); ++i) {
        fmt::println("{}", qubits[i]);
    }
    fmt::println("");
}

// SECTION - Class Operation Member Functions

/**
 * @brief Print overloading
 *
 * @param os
 * @param op
 * @return ostream&
 */
std::ostream& operator<<(std::ostream& os, Operation const& op) {
    return os << fmt::format("{}", op);
}

/**
 * @brief Construct a new Operation:: Operation object
 *
 * @param oper
 * @param ph
 * @param qs
 * @param du
 */
Operation::Operation(GateRotationCategory op, dvlab::Phase ph, std::tuple<size_t, size_t> qubits, std::tuple<size_t, size_t> duration)
    : _operation(op), _phase(ph), _qubits(qubits), _duration(duration) {
    // sort qs
    size_t a = std::get<0>(qubits);
    size_t b = std::get<1>(qubits);
    assert(a != b);
}

}  // namespace qsyn::device
