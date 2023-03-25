/****************************************************************************
  FileName     [ duostra.cpp ]
  PackageName  [ duostra ]
  Synopsis     [ Define class Duostra member functions ]
  Author       [ Chin-Yi Cheng, Chien-Yi Yang, Ren-Chu Wang, Yi-Hsiang Kuo ]
  Paper        [ https://arxiv.org/abs/2210.01306 ]
  Copyright    [ Copyright(c) 2023 DVLab, GIEE, NTU, Taiwan ]
****************************************************************************/

#include "duostra.h"

#include "variables.h"

using namespace std;
extern size_t verbose;

// SECTION - Global settings for Duostra mapper

size_t DUOSTRA_SCHEDULER = 4;            // 0:base 1:static 2:random 3:greedy 4:search
size_t DUOSTRA_ROUTER = 1;               // 0:apsp 1:duostra
size_t DUOSTRA_PLACER = 2;               // 0:static 1:random 2:dfs
bool DUOSTRA_ORIENT = 1;                 // t/f smaller logical qubit index with little priority
size_t DUOSTRA_CANDIDATES = (size_t)-1;  // top k candidates, -1: all
size_t DUOSTRA_APSP_COEFF = 1;           // coefficient of apsp cost
bool DUOSTRA_AVAILABLE = 1;              // 0:min 1:max, available time of double-qubit gate is set to min or max of occupied time
bool DUOSTRA_COST = 0;                   // 0:min 1:max, select min or max cost from the waitlist
size_t DUOSTRA_DEPTH = 5;                // depth of searching region
bool DUOSTRA_NEVER_CACHE = 1;            // never cache any children unless children() is called
bool DUOSTRA_EXECUTE_SINGLE = 1;         // execute the single gates when they are available

extern size_t verbose;

/**
 * @brief Get the Scheduler Type Str object
 *
 * @return string
 */
string getSchedulerTypeStr() {
    // 0:base 1:static 2:random 3:greedy 4:search
    if (DUOSTRA_SCHEDULER == 0) return "base";
    if (DUOSTRA_SCHEDULER == 1) return "static";
    if (DUOSTRA_SCHEDULER == 2) return "random";
    if (DUOSTRA_SCHEDULER == 3) return "greedy";
    if (DUOSTRA_SCHEDULER == 4)
        return "search";
    else
        return "Error";
}

/**
 * @brief Get the Router Type Str object
 *
 * @return string
 */
string getRouterTypeStr() {
    // 0:apsp 1:duostra
    if (DUOSTRA_ROUTER == 0) return "apsp";
    if (DUOSTRA_ROUTER == 1)
        return "duostra";
    else
        return "Error";
}

/**
 * @brief Get the Placer Type Str object
 *
 * @return string
 */
string getPlacerTypeStr() {
    // 0:static 1:random 2:dfs
    if (DUOSTRA_PLACER == 0) return "static";
    if (DUOSTRA_PLACER == 1) return "random";
    if (DUOSTRA_PLACER == 2)
        return "dfs";
    else
        return "Error";
}

/**
 * @brief Construct a new Duostra:: Duostra object
 *
 * @param cir
 * @param dev
 */
Duostra::Duostra(QCir* cir, Device dev) : _logicalCircuit(cir), _physicalCircuit(new QCir(0)), _device(dev) {
}

/**
 * @brief Make dependency graph
 *
 */
void Duostra::makeDependency() {
    vector<Gate> allGates;
    for (const auto& g : _logicalCircuit->getGates()) {
        size_t id = g->getId();

        GateType type = g->getType();

        size_t q2 = ERROR_CODE;
        BitInfo first = g->getQubits()[0];
        BitInfo second = {};
        if (g->getQubits().size() > 1) {
            second = g->getQubits()[1];
            q2 = second._qubit;
        }

        tuple<size_t, size_t> temp{first._qubit, q2};
        Gate tempGate{id, type, g->getPhase(), temp};
        if (first._parent != nullptr) tempGate.addPrev(first._parent->getId());
        if (first._child != nullptr) tempGate.addNext(first._child->getId());
        if (g->getQubits().size() > 1) {
            if (second._parent != nullptr) tempGate.addPrev(second._parent->getId());
            if (second._child != nullptr) tempGate.addNext(second._child->getId());
        }
        allGates.push_back(move(tempGate));
    }
    _dependency = make_shared<DependencyGraph>(_logicalCircuit->getNQubit(), move(allGates));
}

/**
 * @brief Main flow of Duostra mapper
 *
 * @return size_t
 */
size_t Duostra::flow() {
    if (verbose > 3) cout << "Creating dependency of quantum circuit..." << endl;
    makeDependency();
    unique_ptr<CircuitTopo> topo;
    topo = make_unique<CircuitTopo>(_dependency);

    if (verbose > 3) cout << "Creating device..." << endl;
    if (topo->getNumQubits() > _device.getNQubit()) {
        cerr << "You cannot assign more qubits than the device." << endl;
        abort();
    }

    if (verbose > 3) cout << "Initial placing..." << endl;
    auto placer = getPlacer();
    placer->placeAndAssign(_device);

    // scheduler
    if (verbose > 3) cout << "Creating Scheduler..." << endl;
    auto sched = getScheduler(move(topo));

    // router
    if (verbose > 3) cout << "Creating Router..." << endl;
    string cost = (DUOSTRA_SCHEDULER == 3) ? "end" : "start";
    auto router = make_unique<Router>(move(_device), cost, DUOSTRA_ORIENT);

    // routing
    cout << "Routing..." << endl;
    sched->assignGatesAndSort(move(router));

    cout << "Duostra Result: " << endl;
    cout << endl;
    cout << "Scheduler:      " << getSchedulerTypeStr() << endl;
    cout << "Router:         " << getRouterTypeStr() << endl;
    cout << "Placer:         " << getPlacerTypeStr() << endl;
    cout << endl;
    cout << "Mapping Depth:  " << sched->getFinalCost() << "\n";
    cout << "Total Time:     " << sched->getTotalTime() << "\n";
    cout << "#SWAP:          " << sched->getSwapNum() << "\n";
    cout << endl;

    assert(sched->isSorted());
    _result = sched->getOperations();
    buildCircuitByResult();
    cout.clear();
    return sched->getFinalCost();
}

/**
 * @brief Print as qasm form
 *
 */
void Duostra::printAssembly() const {
    cout << "Mapping Result: " << endl;
    cout << endl;
    for (size_t i = 0; i < _result.size(); ++i) {
        const auto& op = _result.at(i);
        string gateName{gateType2Str[op.getType()]};
        cout << left << setw(5) << gateName << " ";
        tuple<size_t, size_t> qubits = op.getQubits();
        string res = "q[" + to_string(get<0>(qubits)) + "]";
        if (get<1>(qubits) != ERROR_CODE) {
            res = res + ",q[" + to_string(get<1>(qubits)) + "]";
        }
        res += ";";
        cout << left << setw(20) << res;
        cout << " // (" << op.getOperationTime() << "," << op.getCost() << ")\n";
    }
}

/**
 * @brief Construct physical QCir by operation
 *
 */
void Duostra::buildCircuitByResult() {
    _physicalCircuit->addQubit(_device.getNQubit());
    for (const auto& operation : _result) {
        string gateName{gateType2Str[operation.getType()]};
        tuple<size_t, size_t> qubits = operation.getQubits();
        vector<size_t> qu;
        qu.emplace_back(get<0>(qubits));
        if (get<1>(qubits) != ERROR_CODE) {
            qu.emplace_back(get<1>(qubits));
        }
        if (operation.getType() == GateType::SWAP) {
            // NOTE - Decompose SWAP into three CX
            vector<size_t> quReverse;
            quReverse.emplace_back(get<1>(qubits));
            quReverse.emplace_back(get<0>(qubits));
            _physicalCircuit->addGate("CX", qu, Phase(0), true);
            _physicalCircuit->addGate("CX", quReverse, Phase(0), true);
            _physicalCircuit->addGate("CX", qu, Phase(0), true);
        } else
            _physicalCircuit->addGate(gateName, qu, operation.getPhase(), true);
    }
}
