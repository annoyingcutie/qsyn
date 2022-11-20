/****************************************************************************
  FileName     [ qcir.cpp ]
  PackageName  [ qcir ]
  Synopsis     [ Define QCir Action functions ]
  Author       [ Chin-Yi Cheng ]
  Copyright    [ Copyleft(c) 2022-present DVLab, GIEE, NTU, Taiwan ]
****************************************************************************/

#include "qcir.h"

#include <algorithm>
#include <cassert>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;
extern size_t verbose;

/// @brief copy a circuit
/// @return QCir*
QCir* QCir::copy(){
    updateTopoOrder();
    QCir* newCircuit = new QCir(0);
    unordered_map<QCirQubit*, QCirQubit*> oldQ2newQ;
    unordered_map<QCirGate*, QCirGate*> oldG2newG;
    newCircuit->addQubit(_qubits.size());
    
    size_t biggestQubit = 0;
    size_t biggestGate = 0;

    for(size_t i=0; i<_qubits.size(); i++){
        oldQ2newQ[_qubits[i]] = newCircuit->getQubits()[i];
        //NOTE - Update Id
        if(_qubits[i]->getId() > biggestQubit)  biggestQubit = _qubits[i]->getId();
        oldQ2newQ[_qubits[i]]->setId(_qubits[i]->getId());
    }
    
    for(const auto& gate: _topoOrder){
        string type = gate->getTypeStr();
        vector<size_t> bits;
        for(const auto& b: gate->getQubits()){
            bits.push_back(b._qubit);
        }
        oldG2newG[gate] = newCircuit->addGate(gate->getTypeStr(), bits, gate->getPhase(), true);
    }

    //NOTE - Update Id
    for(const auto& [oldG, newG]: oldG2newG){
        if(oldG->getId() > biggestGate)  biggestGate = oldG->getId();
        newG->setId(oldG->getId());
    }
    newCircuit->setNextGateId(biggestGate+1);
    newCircuit->setNextQubitId(biggestQubit+1);
    return newCircuit;
}

QCir* QCir::compose(QCir* target){
    cerr << "Compose is not provided yet!!";
    return nullptr;
}

QCir* QCir::tensorProduct(QCir* target){
    cerr << "Tensor is not provided yet!!";
    return nullptr;
}

/**
 * @brief Perform DFS from currentGate
 *
 * @param currentGate
 */
void QCir::DFS(QCirGate *currentGate) {
    currentGate->setVisited(_globalDFScounter);

    vector<BitInfo> Info = currentGate->getQubits();
    for (size_t i = 0; i < Info.size(); i++) {
        if ((Info[i])._child != NULL) {
            if (!((Info[i])._child->isVisited(_globalDFScounter))) {
                DFS((Info[i])._child);
            }
        }
    }
    _topoOrder.push_back(currentGate);
}

/**
 * @brief Update Topological Order
 */
void QCir::updateTopoOrder() {
    _topoOrder.clear();
    _globalDFScounter++;
    QCirGate *dummy = new HGate(-1);

    for (size_t i = 0; i < _qubits.size(); i++) {
        dummy->addDummyChild(_qubits[i]->getFirst());
    }
    DFS(dummy);
    _topoOrder.pop_back();  // pop dummy
    reverse(_topoOrder.begin(), _topoOrder.end());
    assert(_topoOrder.size() == _qgates.size());
}

/**
 * @brief Print topological order
 */
bool QCir::printTopoOrder() {
    auto testLambda = [](QCirGate *G) { cout << G->getId() << endl; };
    topoTraverse(testLambda);
    return true;
}

/**
 * @brief Update execution time of gates
 */
void QCir::updateGateTime() {
    auto Lambda = [](QCirGate *currentGate) {
        vector<BitInfo> Info = currentGate->getQubits();
        size_t max_time = 0;
        for (size_t i = 0; i < Info.size(); i++) {
            if (Info[i]._parent == NULL)
                continue;
            if (Info[i]._parent->getTime() + 1 > max_time)
                max_time = Info[i]._parent->getTime() + 1;
        }
        currentGate->setTime(max_time);
    };
    topoTraverse(Lambda);
}

/**
 * @brief Print ZX-graph of gate following the topological order
 */
void QCir::printZXTopoOrder() {
    auto Lambda = [this](QCirGate *G) {
        cout << "Gate " << G->getId() << " (" << G->getTypeStr() << ")" << endl;
        ZXGraph *tmp = G->getZXform();
        tmp->printVertices();
    };
    topoTraverse(Lambda);
}

void QCir::reset(){
    _qgates.clear();
    _qubits.clear();
    _topoOrder.clear();
    _ZXGraphList.clear();
    _qubit2pin.clear();
    
    _gateId = 0;
    _ZXNodeId = 0;
    _qubitId = 0;
    _dirty = true;
    _globalDFScounter = 1;
    _tensor = nullptr;
}