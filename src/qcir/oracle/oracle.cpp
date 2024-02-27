/****************************************************************************
  PackageName  [ qcir/oracle ]
  Synopsis     [ Define functions for boolean oracle synthesis ]
  Author       [ Design Verification Lab ]
  Paper        [ https://arxiv.org/abs/1904.02121 ]
  Copyright    [ Copyright(c) 2023 DVLab, GIEE, NTU, Taiwan ]
****************************************************************************/

#include "qcir/oracle/oracle.hpp"

#include <algorithm>
#include <ranges>
#include <sstream>
#include <tl/enumerate.hpp>
#include <tl/to.hpp>
#include <tl/zip.hpp>
#include <utility>
#include <vector>

#include "qcir/oracle/k_lut.hpp"
#include "qcir/oracle/pebble.hpp"
#include "qcir/oracle/xag.hpp"
#include "util/sat/sat_solver.hpp"

namespace views = std::views;

namespace qsyn::qcir {

std::optional<QCir> synthesize_boolean_oracle(XAG xag, size_t n_ancilla, size_t k) {
    size_t const P               = n_ancilla + xag.outputs.size();
    auto const& [optimal_cut, _] = k_lut_partition(xag, k);
    fmt::print("P = {}, k = {}\n", P, k);

    // renumber the nodes in the optimal cut
    std::map<XAGNodeID, DepGraphNodeID> xag_to_deps;
    std::map<DepGraphNodeID, XAGNodeID> deps_to_xag;
    std::vector<XAGNodeID> optimal_cone_tips = optimal_cut |
                                               views::keys |
                                               tl::to<std::vector>();
    std::sort(optimal_cone_tips.begin(), optimal_cone_tips.end());
    std::stringstream dep_graph_ss;
    for (auto const& [i, node_id] : tl::views::enumerate(optimal_cone_tips)) {
        xag_to_deps[node_id]           = DepGraphNodeID(i);
        deps_to_xag[DepGraphNodeID(i)] = node_id;
    }
    dep_graph_ss << fmt::format("{}", fmt::join(xag.outputs | views::transform([&](auto& xag_id) { return xag_to_deps.at(xag_id).get(); }), " ")) << '\n';
    for (auto i : views::iota(0ul, optimal_cone_tips.size())) {
        auto const dep_id    = DepGraphNodeID(i);
        auto const xag_id    = deps_to_xag.at(dep_id);
        auto const& xag_node = xag.get_node(xag_id);
        dep_graph_ss << fmt::format("{}", i);
        if (xag_node->get_type() == XAGNodeType::INPUT) {
            dep_graph_ss << '\n';
            continue;
        }
        for (auto const& deps : optimal_cut.at(xag_id)) {
            dep_graph_ss << ' ' << fmt::format("{}", xag_to_deps.at(deps).get());
        }
        dep_graph_ss << '\n';
    }

    auto graph = create_dependency_graph(dep_graph_ss);

    const size_t N        = graph.size();  // number of nodes
    const size_t max_deps = std::ranges::max(graph.get_graph() |
                                             std::views::values |
                                             std::views::transform([](DepGraphNode const& node) {
                                                 return node.dependencies.size();
                                             }));
    const size_t _P       = sanitize_P(P, N, max_deps);

    dvlab::sat::CaDiCalSolver solver{};
    auto schedule = pebble(solver, _P, graph);

    fmt::println("solution:");
    for (const size_t i : views::iota(0UL, schedule->size())) {
        fmt::println("time = {:02} : {}", i, fmt::join((*schedule)[i] | views::transform([](bool b) { return b ? "*" : "."; }), ""));
    }

    // TODO: actually synthesize the oracle
    return std::nullopt;
}

}  // namespace qsyn::qcir
