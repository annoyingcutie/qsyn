/****************************************************************************
  FileName     [ mappingEQChecker.h ]
  PackageName  [ duostra ]
  Synopsis     [ Define class MappingEQChecker structure ]
  Author       [ Design Verification Lab ]
  Copyright    [ Copyright(c) 2023 DVLab, GIEE, NTU, Taiwan ]
****************************************************************************/

#pragma once

#include <cstddef>
#include <optional>

#include "device/device.hpp"

class QCir;

class MappingEQChecker {
public:
    MappingEQChecker(QCir*, QCir*, Device, std::vector<size_t> = {}, bool = false);

    bool check();
    bool isSwap(QCirGate*);
    bool executeSwap(QCirGate*, std::unordered_set<QCirGate*>&);
    bool executeSingle(QCirGate*);
    bool executeDouble(QCirGate*);

    bool checkRemain();
    QCirGate* getNext(const BitInfo&);

private:
    QCir* _physical;
    QCir* _logical;
    Device _device;
    bool _reverse;
    // <qubit, gate to execute (from back)> for logical circuit
    std::unordered_map<size_t, QCirGate*> _dependency;
};

