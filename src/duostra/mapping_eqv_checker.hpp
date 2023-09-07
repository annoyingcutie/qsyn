/****************************************************************************
  PackageName  [ duostra ]
  Synopsis     [ Define class MappingEquivalenceChecker structure ]
  Author       [ Design Verification Lab ]
  Copyright    [ Copyright(c) 2023 DVLab, GIEE, NTU, Taiwan ]
****************************************************************************/

#pragma once

#include <cstddef>
#include <optional>
#include <unordered_set>

#include "device/device.hpp"

class QCir;
class QCirGate;
class QubitInfo;

class MappingEquivalenceChecker {
public:
    MappingEquivalenceChecker(QCir*, QCir*, Device, std::vector<size_t> = {}, bool = false);

    bool check();
    bool is_swap(QCirGate*);
    bool execute_swap(QCirGate*, std::unordered_set<QCirGate*>&);
    bool execute_single(QCirGate*);
    bool execute_double(QCirGate*);

    bool check_remaining();
    QCirGate* get_next(QubitInfo const&);

private:
    QCir* _physical;
    QCir* _logical;
    Device _device;
    bool _reverse;
    // <qubit, gate to execute (from back)> for logical circuit
    std::unordered_map<size_t, QCirGate*> _dependency;
};
