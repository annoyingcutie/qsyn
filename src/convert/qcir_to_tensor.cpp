/****************************************************************************
  PackageName  [ qsyn ]
  Synopsis     [ Define class QCir Mapping functions ]
  Author       [ Design Verification Lab ]
  Copyright    [ Copyright(c) 2023 DVLab, GIEE, NTU, Taiwan ]
****************************************************************************/

#include "./qcir_to_tensor.hpp"

#include <spdlog/spdlog.h>

#include <cstddef>
#include <gsl/narrow>
#include <stdexcept>
#include <thread>

#include "fmt/core.h"
#include "qcir/gate_type.hpp"
#include "qcir/qcir.hpp"
#include "qcir/qcir_qubit.hpp"
#include "qsyn/qsyn_type.hpp"
#include "tensor/qtensor.hpp"

extern bool stop_requested();

namespace qsyn {

using Qubit2TensorPinMap = std::unordered_map<QubitIdType, std::pair<size_t, size_t>>;
using QubitReorderingMap = std::unordered_map<QubitIdType, QubitIdType>;

using qsyn::tensor::QTensor;

/**
 * @brief Convert gate to tensor
 *
 * @param gate
 * @return std::optional<QTensor<double>>
 */
std::optional<QTensor<double>> to_tensor(QCirGate *gate) {
    switch (gate->get_rotation_category()) {
        // single-qubit gates
        case GateRotationCategory::id:
            return QTensor<double>::identity(1);
        case GateRotationCategory::h:
            return QTensor<double>::hbox(2);
        case GateRotationCategory::swap: {
            auto tensor = QTensor<double>{{1.0, 0.0, 0.0, 0.0},
                                          {0.0, 0.0, 1.0, 0.0},
                                          {0.0, 1.0, 0.0, 0.0},
                                          {0.0, 0.0, 0.0, 1.0}};
            tensor.reshape({2, 2, 2, 2});
            return tensor;
        }
        case GateRotationCategory::pz:
            return QTensor<double>::control(QTensor<double>::pzgate(gate->get_phase()), gate->get_num_qubits() - 1);
        case GateRotationCategory::rz:
            return QTensor<double>::control(QTensor<double>::rzgate(gate->get_phase()), gate->get_num_qubits() - 1);
        case GateRotationCategory::px:
            return QTensor<double>::control(QTensor<double>::pxgate(gate->get_phase()), gate->get_num_qubits() - 1);
        case GateRotationCategory::rx:
            return QTensor<double>::control(QTensor<double>::rxgate(gate->get_phase()), gate->get_num_qubits() - 1);
        case GateRotationCategory::py:
            return QTensor<double>::control(QTensor<double>::pygate(gate->get_phase()), gate->get_num_qubits() - 1);
        case GateRotationCategory::ry:
            return QTensor<double>::control(QTensor<double>::rygate(gate->get_phase()), gate->get_num_qubits() - 1);

        default:
            return std::nullopt;
    }
};

namespace {

/**
 * @brief Update tensor pin
 *
 * @param qubit2pin map of reordering qubit to pin
 * @param reordering_map qubit reordering
 * @param pins gate pins
 * @param gate new gate
 * @param main main tensor
 */
void update_tensor_pin(Qubit2TensorPinMap &qubit2pin, QubitReorderingMap &reordering_map, std::vector<QubitInfo> const &qubit_infos, QTensor<double> const &gate, QTensor<double> &main) {
    spdlog::trace("Pin Permutation");
    for (auto &[qubit, pin] : qubit2pin) {
        auto const [old_out, old_in] = pin;
        auto &[new_out, new_in]      = pin;

        auto const it = std::ranges::find_if(qubit_infos, [qubit = reordering_map[qubit]](QubitInfo const &info) { return info._qubit == qubit; });

        if (it != qubit_infos.end()) {
            auto ith_ctrl = std::distance(qubit_infos.begin(), it);
            new_out       = main.get_new_axis_id(2 * ith_ctrl);
        } else {
            new_out = main.get_new_axis_id(gate.dimension() + old_out);
        }
        // NOTE - Order of axis [ Gate ctrl 0 in, Gate ctrl 0 out, .... , Gate targ in, Gate targ out, Tensor 1 in, Tensor 1 out, ...]
        new_in = main.get_new_axis_id(gate.dimension() + old_in);
        spdlog::trace("  - Qubit: {} input : {} -> {} output: {} -> {}", qubit, old_out, new_out, old_in, new_in);
    }
}

}  // namespace

/**
 * @brief Convert QCir to tensor
 */
std::optional<QTensor<double>> to_tensor(QCir const &qcir) try {
    if (qcir.get_qubits().empty()) {
        spdlog::warn("QCir is empty!!");
        return std::nullopt;
    }
    qcir.update_topological_order();
    spdlog::debug("Add boundary");

    QTensor<double> tensor;

    // NOTE: Constucting an identity(_qubit.size()) takes much time and memory.
    //       To make this process interruptible by SIGINT (ctrl-C), we grow the qubit size one by one
    for (size_t i = 0; i < qcir.get_qubits().size(); ++i) {
        if (stop_requested()) {
            spdlog::warn("Conversion interrupted.");
            return std::nullopt;
        }
        tensor = tensordot(tensor, QTensor<double>::identity(1));
    }

    // NOTE - Reordering qubits
    std::vector<QubitIdType> id_list;
    for (const auto qb : qcir.get_qubits())
        id_list.emplace_back(qb->get_id());
    std::sort(id_list.begin(), id_list.end());
    QubitReorderingMap reordered_qubit_ids;
    for (size_t i = 0; i < id_list.size(); i++) {
        reordered_qubit_ids[id_list[i]] = gsl::narrow<QubitIdType>(i);
    }

    Qubit2TensorPinMap qubit_to_pins;
    for (size_t i = 0; i < qcir.get_qubits().size(); i++) {
        auto reordered_id           = reordered_qubit_ids[qcir.get_qubits()[i]->get_id()];
        qubit_to_pins[reordered_id] = std::make_pair(2 * reordered_id, 2 * reordered_id + 1);
        spdlog::trace("  - Add Qubit {} input port: {}", 2 * reordered_id, 2 * reordered_id + 1);
    }

    try {
        qcir.topological_traverse([&tensor, &qubit_to_pins, &reordered_qubit_ids](QCirGate *gate) {
            if (stop_requested()) return;
            spdlog::debug("Gate {} ({})", gate->get_id(), gate->get_type_str());
            auto gate_tensor = to_tensor(gate);
            if (!gate_tensor.has_value()) {
                throw std::runtime_error(fmt::format("Gate {} ({}) is not supported!!", gate->get_id(), gate->get_type_str()));
            }
            std::vector<size_t> main_tensor_output_pins;
            std::vector<size_t> gate_tensor_input_pins;
            for (size_t np = 0; np < gate->get_qubits().size(); np++) {
                gate_tensor_input_pins.emplace_back(2 * np + 1);
                auto const info = gate->get_qubits()[np];
                main_tensor_output_pins.emplace_back(qubit_to_pins[reordered_qubit_ids[info._qubit]].first);
            }
            // [tmp]x[tensor]
            tensor = tensordot(*gate_tensor, tensor, gate_tensor_input_pins, main_tensor_output_pins);
            update_tensor_pin(qubit_to_pins, reordered_qubit_ids, gate->get_qubits(), *gate_tensor, tensor);
        });

    } catch (std::runtime_error const &e) {
        spdlog::error("{}", e.what());
        return std::nullopt;
    }

    if (stop_requested()) {
        spdlog::warn("Conversion interrupted.");
        return std::nullopt;
    }

    std::vector<size_t> output_pins, input_pins;
    for (size_t i = 0; i < qcir.get_qubits().size(); i++) {
        output_pins.emplace_back(qubit_to_pins[qcir.get_qubits()[i]->get_id()].first);
        input_pins.emplace_back(qubit_to_pins[qcir.get_qubits()[i]->get_id()].second);
    }

    tensor = tensor.to_matrix(output_pins, input_pins);

    return tensor;
} catch (std::bad_alloc const &e) {
    spdlog::error("Memory allocation failed!!");
    return std::nullopt;
}

}  // namespace qsyn
