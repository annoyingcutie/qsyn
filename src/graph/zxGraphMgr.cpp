/****************************************************************************
  FileName     [ zxGraphMgr.cpp ]
  PackageName  [ graph ]
  Synopsis     [ Define ZX-graph manager ]
  Author       [ Cheng-Hua Lu ]
  Copyright    [ Copyleft(c) 2022-present DVLab, GIEE, NTU, Taiwan ]
****************************************************************************/

#include "zxGraphMgr.h"

#include <iostream>
#include <vector>
using namespace std;

ZXGraphMgr* zxGraphMgr = 0;
extern size_t verbose;

/*****************************************/
/*   class ZXGraphMgr member functions   */
/*****************************************/

void ZXGraphMgr::reset() {
    _graphList.clear();
    _gListItr = _graphList.begin();
    _nextID = 0;
}

// Test

bool ZXGraphMgr::isID(size_t id) const {
    for (size_t i = 0; i < _graphList.size(); i++) {
        if (_graphList[i]->getId() == id) return true;
    }
    return false;
}

// Add and Remove

ZXGraph* ZXGraphMgr::addZXGraph(size_t id, void** ref) {
    ZXGraph* zxGraph = new ZXGraph(id, ref);
    _graphList.push_back(zxGraph);
    _gListItr = _graphList.end() - 1;
    if (id == _nextID || _nextID < id) _nextID = id + 1;
    if (verbose >= 3) {
        cout << "Successfully generated Graph " << id << endl;
        cout << "Checkout to Graph " << id << endl;
    }
    return zxGraph;
}

void ZXGraphMgr::removeZXGraph(size_t id) {
    for (size_t i = 0; i < _graphList.size(); i++) {
        if (_graphList[i]->getId() == id) {
            ZXGraph* rmGraph = _graphList[i];
            rmGraph->setRef(NULL);
            _graphList.erase(_graphList.begin() + i);
            delete rmGraph;
            if (verbose >= 3) cout << "Successfully removed Graph " << id << endl;
            _gListItr = _graphList.begin();
            if (verbose >= 3) {
                if (!_graphList.empty())
                    cout << "Checkout to Graph " << _graphList[0]->getId() << endl;
                else
                    cout << "Note: The graph list is empty now" << endl;
            }
            return;
        }
    }
    cerr << "Error: The id provided does not exist!!" << endl;
    return;
}

// Action

void ZXGraphMgr::checkout2ZXGraph(size_t id) {
    for (size_t i = 0; i < _graphList.size(); i++) {
        if (_graphList[i]->getId() == id) {
            _gListItr = _graphList.begin() + i;
            if (verbose >= 3) cout << "Checkout to Graph " << id << endl;
            return;
        }
    }
    cerr << "Error: The id provided does not exist!!" << endl;
    return;
}

void ZXGraphMgr::copy(size_t id) {
    if (_graphList.empty())
        cerr << "Error: ZXGraphMgr is empty now! Action \"copy\" failed!" << endl;
    else {
        bool exists = false;
        ZXGraph* copyTarget = getGraph()->copy();
        copyTarget->setId(id);

        // Overwrite existing ZXGraph
        for (size_t i = 0; i < _graphList.size(); i++) {
            if (_graphList[i]->getId() == id) {
                cout << "Overwrite existing Graph " << id << endl; // TODO guard this action?
                _graphList.erase(_graphList.begin() + i);
                _graphList.insert(_graphList.begin() + i, copyTarget);
                if (verbose >= 3) cout << "Successfully copied Graph " << getGraph()->getId() << " to Graph " << id << endl;
                checkout2ZXGraph(id);
                exists = true;
                break;
            }
        }
        // Create a new ZXGraph
        if (!exists) {
            size_t oriGraphID = getGraph()->getId();
            _graphList.push_back(copyTarget);
            _gListItr = _graphList.end() - 1;
            if (id == _nextID || _nextID < id) _nextID = id + 1;
            if (verbose >= 3) {
                cout << "Successfully copied Graph " << oriGraphID << " to Graph " << id << endl;
                cout << "Checkout to Graph " << id << endl;
            }
        }
    }
}

void ZXGraphMgr::compose(ZXGraph* zxGraph) {
    ZXGraph* oriGraph = getGraph();
    // oriGraph->sortIOByQubit();
    if (oriGraph->getNumOutputs() != zxGraph->getNumInputs())
        cerr << "Error: The composing ZX-graph's #input is not equivalent to the original ZX-graph's #output." << endl;
    else {
        // Make a deep copy of zxGraph
        ZXGraph* copyGraph = zxGraph->copy();
        // copyGraph->sortIOByQubit();

        // Rewrite all vertices id in copyGraph to avoid repetition
        size_t nextID = oriGraph->findNextId();
        for (size_t i = 0; i < copyGraph->getVertices().size(); i++) {
            copyGraph->getVertices()[i]->setId(nextID);
            nextID++;
        }

        //! TODO
        // Connect each vertex that originally connected to output of oriGraph to each vertex that originally coneected to input of copyGraph

        for (size_t i = 0; i < oriGraph->getOutputs().size(); i++) {
            ZXVertex* curOut = oriGraph->getOutputs()[i];
            ZXVertex* cpIn = copyGraph->getInputs()[i];
            NeighborMap curOutNMap = curOut->getNeighborMap();
            NeighborMap cpInNMap = cpIn->getNeighborMap();
            for (auto& s : curOutNMap) {
                ZXVertex* vs = s.first;
                EdgeType* vsType = s.second;
                for (auto& t : cpInNMap) {
                    ZXVertex* vt = t.first;
                    EdgeType* vtType = t.second;
                    EdgeType* newType;
                    if ((*vsType == EdgeType::SIMPLE && *vtType == EdgeType::SIMPLE) || (*vsType == EdgeType::HADAMARD && *vtType == EdgeType::HADAMARD))
                        newType = new EdgeType(EdgeType::SIMPLE);
                    else
                        newType = new EdgeType(EdgeType::HADAMARD);
                    oriGraph->addEdge(vs, vt, newType);
                    vt->disconnect(cpIn);
                }
                vs->disconnect(curOut);
            }
        }
        // Remove outputs of oriGraph and inputs of copyGraph
        oriGraph->removeVertices(oriGraph->getOutputs(), true);
        copyGraph->removeVertices(copyGraph->getInputs(), true);

        // Update data in oriGraph
        oriGraph->setOutputs(copyGraph->getOutputs());
        oriGraph->setOutputList(copyGraph->getOutputList());
        oriGraph->addVertices(copyGraph->getVertices());
        oriGraph->addEdges(copyGraph->getEdges());
    }
}

void ZXGraphMgr::tensorProduct(ZXGraph* zxGraph) {
    ZXGraph* oriGraph = getGraph();
    ZXGraph* copyGraph = zxGraph->copy();
    size_t liftNum = max(oriGraph->getNumInputs(), oriGraph->getNumOutputs());
    copyGraph->liftQubit(liftNum);    

    // Rewrite all vertices id in copyGraph to avoid repetition
    size_t nextID = oriGraph->findNextId();
    for (size_t i = 0; i < copyGraph->getVertices().size(); i++) {
        copyGraph->getVertices()[i]->setId(nextID);
        nextID++;
    }

    // Add inputs / outputs / vertices / edges
    oriGraph->addInputs(copyGraph->getInputs());
    oriGraph->addOutputs(copyGraph->getOutputs());
    oriGraph->addVertices(copyGraph->getVertices());
    oriGraph->addEdges(copyGraph->getEdges());
    oriGraph->mergeInputList(copyGraph->getInputList());
    oriGraph->mergeOutputList(copyGraph->getOutputList());
    
}

ZXGraph* ZXGraphMgr::findZXGraphByID(size_t id) const {
    if (!isID(id))
        cerr << "Error: Graph " << id << " does not exist!" << endl;
    else {
        for (size_t i = 0; i < _graphList.size(); i++) {
            if (_graphList[i]->getId() == id) return _graphList[i];
        }
    }
    return nullptr;
}

// Print
void ZXGraphMgr::printZXGraphMgr() const {
    cout << "-> #Graph: " << _graphList.size() << endl;
    if (!_graphList.empty()) cout << "-> Now focus on: " << getGraph()->getId() << endl;
}

void ZXGraphMgr::printGListItr() const {
    if (!_graphList.empty())
        cout << "Now focus on: " << getGraph()->getId() << endl;
    else
        cerr << "Error: ZXGraphMgr is empty now!" << endl;
}

void ZXGraphMgr::printGraphListSize() const {
    cout << "#Graph: " << _graphList.size() << endl;
}