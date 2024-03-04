/****************************************************************************
  PackageName  [ duostra ]
  Synopsis     [ Define class Checker structure ]
  Author       [ Chin-Yi Cheng, Chien-Yi Yang, Ren-Chu Wang, Yi-Hsiang Kuo ]
  Paper        [ https://arxiv.org/abs/2210.01306 ]
  Copyright    [ Copyright(c) 2023 DVLab, GIEE, NTU, Taiwan ]
****************************************************************************/

#pragma once

#include "device/device.hpp"
#include "qsyn/qsyn_type.hpp"

namespace qsyn::duostra {

class CircuitTopology;
class Gate;

class Checker {
public:
    using Device        = qsyn::device::Device;
    using Operation     = qsyn::device::Operation;
    using PhysicalQubit = qsyn::device::PhysicalQubit;
    Checker(CircuitTopology& topo,
            Checker::Device& device,
            std::span<Checker::Operation const> ops,
            std::vector<QubitIdType> const& assign, bool tqdm = true);

    size_t get_cycle(Operation const& op);

    void apply_gate(Operation const& op, PhysicalQubit& q0);
    void apply_gate(Operation const& op, PhysicalQubit& q0, PhysicalQubit& q1);
    void apply_swap(Operation const& op);
    bool apply_cx(Operation const& op, Gate const& gate);
    bool apply_single(Operation const& op, Gate const& gate);

    bool test_operations();

private:
    CircuitTopology* _topo;
    Device* _device;
    std::span<Operation const> _ops;
    bool _tqdm;
};

}  // namespace qsyn::duostra
