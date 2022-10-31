/****************************************************************************
  FileName     [ zxGraph.h ]
  PackageName  [ graph ]
  Synopsis     [ Define class ZXGraph member functions ]
  Author       [ Cheng-Hua Lu ]
  Copyright    [ Copyleft(c) 2022-present DVLab, GIEE, NTU, Taiwan ]
****************************************************************************/

#include "zxGraph.h"

#include <algorithm>
#include <cassert>
#include <iomanip>
#include <iostream>
#include <vector>
#include <unordered_set>

#include "util.h"
#include "textFormat.h"
#include <ranges>
#include <chrono>

using namespace std;
namespace TF = TextFormat;
extern size_t verbose;

EdgeType toggleEdge(const EdgeType& et) {
    if (et == EdgeType::SIMPLE)
        return EdgeType::HADAMARD;
    else if (et == EdgeType::HADAMARD)
        return EdgeType::SIMPLE;
    else
        return EdgeType::ERRORTYPE;
}

VertexType str2VertexType(const string& str) {
    if (str == "BOUNDARY")
        return VertexType::BOUNDARY;
    else if (str == "Z")
        return VertexType::Z;
    else if (str == "X")
        return VertexType::X;
    else if (str == "H_BOX")
        return VertexType::H_BOX;
    return VertexType::ERRORTYPE;
}

string VertexType2Str(const VertexType& vt) {
    if (vt == VertexType::X) return TF::BOLD(TF::RED("X"));
    if (vt == VertexType::Z) return TF::BOLD(TF::GREEN("Z"));
    if (vt == VertexType::H_BOX) return TF::BOLD(TF::YELLOW("H"));
    if (vt == VertexType::BOUNDARY) return "●";
    return "";
}

EdgeType* str2EdgeType(const string& str) {
    if (str == "SIMPLE")
        return new EdgeType(EdgeType::SIMPLE);
    else if (str == "HADAMARD")
        return new EdgeType(EdgeType::HADAMARD);
    return new EdgeType(EdgeType::ERRORTYPE);
}

string EdgeType2Str(const EdgeType* et) {
    if (*et == EdgeType::SIMPLE) return "-";
    if (*et == EdgeType::HADAMARD) return TF::BOLD(TF::BLUE("H"));
    return "";
}

/**************************************/
/*   class ZXVertex member functions   */
/**************************************/

ZXVertex* ZXVertex::getNeighbor(size_t idx) const {
    if (idx > getNeighbors().size() - 1) return nullptr;
    return getNeighbors()[idx];
}

vector<ZXVertex*> ZXVertex::getNeighbors() const {
    vector<ZXVertex*> neighbors;
    for (auto& itr : getNeighborMap()) neighbors.push_back(itr.first);
    return neighbors;
}

// Print functions

/**
 * @brief Print summary of ZXVertex
 *
 */
void ZXVertex::printVertex() const {
    cout << "ID:\t" << _id << "\t";
    cout << "VertexType:\t" << VertexType2Str(_type) << "\t";
    cout << "Qubit:\t" << _qubit << "\t";
    cout << "Phase:\t" << _phase << "\t";
    cout << "#Neighbors:\t" << _neighborMap.size() << "\t";
    printNeighborMap();
}

/**
 * @brief Print each element in _neighborMap
 *
 */
void ZXVertex::printNeighborMap() const {
    vector<pair<ZXVertex*, EdgeType*> > neighborList;
    for (auto& itr : _neighborMap) neighborList.push_back(make_pair(itr.first, itr.second));

    sort(neighborList.begin(), neighborList.end(), [](pair<ZXVertex*, EdgeType*> a, pair<ZXVertex*, EdgeType*> b) { return a.first->getId() < b.first->getId(); });

    for (size_t i = 0; i < neighborList.size(); i++) {
        cout << "(" << neighborList[i].first->getId() << ", " << EdgeType2Str(neighborList[i].second) << ") ";
    }
    cout << endl;
}

// Action

/**
 * @brief Remove all the connection between `this` and `v`.
 *
 * @param v
 * @param checked
 */
void ZXVertex::disconnect(ZXVertex* v, bool checked) {
    if (!checked) {
        if (!isNeighbor(v)) {
            cerr << "Error: Vertex " << v->getId() << " is not a neighbor of " << _id << endl;
            return;
        }
    }

    _neighborMap.erase(v);
    NeighborMap nMap = v->getNeighborMap();
    nMap.erase(this);
    v->setNeighborMap(nMap);
}

// Test

/**
 * @brief Check if `v` is one of the neighbors in ZXVertex
 *
 * @param v
 * @return bool
 */
bool ZXVertex::isNeighbor(ZXVertex* v) const {
    return _neighborMap.contains(v);
}

/**
 * @brief Check if ZXGraph is graph-like, report first error
 *
 * @param
 * @return bool
 */
bool ZXGraph::isGraphLike() const {
    
    // 2. all Hedge or Bedge
    for(size_t i=0; i < _edges.size(); i++){
        if((*_edges[i].second)== EdgeType::HADAMARD) continue;
        else{
            if(_edges[i].first.first->getType()== VertexType::BOUNDARY || _edges[i].first.second->getType()== VertexType::BOUNDARY) continue;
            else{
                cout << "False: Type (" << *_edges[i].second << ") of edge " << _edges[i].first.first->getId() << "--" << _edges[i].first.second->getId() << " is invalid!!" << endl;
                return false;
            }
        }
    }
    // 4. B-Z-B and B has only an edge
    for(size_t i=0; i<_inputs.size(); i++){
        if(_inputs[i] -> getNumNeighbors() != 1){
            cout << "False: Boundary vertex " << _inputs[i]->getId() << " has invalid number of neighbors!!" << endl;
            return false;
        }
        if(_inputs[i] -> getNeighbor(0) -> getType() == VertexType::BOUNDARY){
            cout << "False: Boundary vertex " << _inputs[i]->getId() << " has a boundary neighbor!!" << _inputs[i] -> getNeighbor(0) -> getId() << " !!" << endl;
            return false;
        }
    }
    // 1. all Z or B  3. no parallel, no selfloop (vertex neighbor)
    for(size_t i=0; i < _vertices.size(); i++){
        if(_vertices[i]->getType()!=VertexType::BOUNDARY && _vertices[i]->getType()!=VertexType::Z){
            cout << "False: Type (" << _vertices[i]->getType() << ") of vertex " << _vertices[i]->getId() << " is invalid!!" << endl;
            return false;
        }
        vector<ZXVertex* > neighbors = _vertices[i]->getNeighbors();
        vector<ZXVertex* > found;
        for(size_t j=0; j<neighbors.size(); j++){
            if(neighbors[j] == _vertices[i]){
                cout << "False: Vertex "<< _vertices[i]->getId() << " has selfloop(s)!!" << endl;
                return false;
            }
            else{
                if(find(found.begin(), found.end(), neighbors[j]) != found.end()){
                    cout << "False: Vertices " << _vertices[i]->getId() << " and " << neighbors[j]->getId() << " have parallel edges!!" << endl;
                    return false;
                }
                found.push_back(neighbors[j]);
            }
        }
    }
    cout << TF::BOLD(TF::GREEN("True: The graph is graph-like")) << endl;
    return true;
}

/**************************************/
/*   class ZXGraph member functions   */
/**************************************/

// Getter and setter

size_t ZXGraph::getNumIncidentEdges(ZXVertex* v) const {
    // cout << "Find incident of " << v->getId() << endl; 
    size_t count = 0;
    for(const auto& edge : _edges){
        if(edge.first.first == v || edge.first.second == v) count++;
    }
    return count;
}

EdgePair ZXGraph::getFirstIncidentEdge(ZXVertex* v) const {
    for(const auto& edge : _edges){
        if(edge.first.first == v || edge.first.second == v) return edge;
    }
    return make_pair(make_pair(nullptr, nullptr), nullptr);
}

vector<EdgePair> ZXGraph::getIncidentEdges(ZXVertex* v) const {
    // cout << "Find incident of " << v->getId() << endl; 
    vector<EdgePair> incidentEdges;
    for(size_t e = 0; e < _edges.size(); e++){
        if(_edges[e].first.first == v || _edges[e].first.second == v){
            // cout << _edges[e].first.first->getId() << " " << _edges[e].first.second->getId() << endl;
            incidentEdges.push_back(_edges[e]);
        }
    }
    return incidentEdges;
}

// For testing
void ZXGraph::generateCNOT() {
    cout << "Generate a 2-qubit CNOT graph for testing" << endl;
    // Generate Inputs
    vector<ZXVertex*> inputs, outputs, vertices;
    for (size_t i = 0; i < 2; i++) {
        addInput(findNextId(), i);
    }

    // Generate CNOT
    addVertex(findNextId(), 0, VertexType::Z);
    addVertex(findNextId(), 1, VertexType::X);

    // Generate Outputs
    for (size_t i = 0; i < 2; i++) {
        addOutput(findNextId(), i);
    }

    // Generate edges [(0,2), (1,3), (2,3), (2,4), (3,5)]
    addEdgeById(3, 5, new EdgeType(EdgeType::SIMPLE));
    addEdgeById(0, 2, new EdgeType(EdgeType::SIMPLE));
    addEdgeById(1, 3, new EdgeType(EdgeType::SIMPLE));
    addEdgeById(2, 3, new EdgeType(EdgeType::SIMPLE));
    addEdgeById(2, 4, new EdgeType(EdgeType::SIMPLE));
}

bool ZXGraph::isEmpty() const {
    if (_inputs.empty() && _outputs.empty() && _vertices.empty() && _edges.empty()) return true;
    return false;
}

bool ZXGraph::isValid() const {
    for (auto v: _inputs) {
        if (v->getNumNeighbors() != 1) return false;
    }
    for (auto v: _outputs) {
        if (v->getNumNeighbors() != 1) return false;
    }
    for (size_t i = 0; i < _edges.size(); i++) {
        if (!_edges[i].first.first->isNeighbor(_edges[i].first.second) ||
            !_edges[i].first.second->isNeighbor(_edges[i].first.first)) return false;
    }
    return true;
}

bool ZXGraph::isConnected(ZXVertex* v1, ZXVertex* v2) const {
    if (v1->isNeighbor(v2) && v2->isNeighbor(v1)) return true;
    return false;
}

bool ZXGraph::isId(size_t id) const {
    for (size_t i = 0; i < _vertices.size(); i++) {
        if (_vertices[i]->getId() == id) return true;
    }
    return false;
}

bool ZXGraph::isInputQubit(int qubit) const {
    return (_inputList.contains(qubit));
}

bool ZXGraph::isOutputQubit(int qubit) const {
    return (_outputList.contains(qubit));
}

// Add and Remove
ZXVertex* ZXGraph::addInput(size_t id, int qubit, bool checked) {
    if (!checked) {
        if (isId(id)) {
            cerr << "Error: This vertex id already exists!!" << endl;
            return nullptr;
        } else if (isInputQubit(qubit)) {
            cerr << "Error: This qubit's input already exists!!" << endl;
            return nullptr;
        }
    }
    ZXVertex* v = new ZXVertex(id, qubit, VertexType::BOUNDARY);
    _inputs.push_back(v);
    _vertices.push_back(v);
    setInputHash(qubit, v);
    if (verbose >= 5) cout << "Add input " << id << endl;
    return v;
}

ZXVertex* ZXGraph::addOutput(size_t id, int qubit, bool checked) {
    if (!checked) {
        if (isId(id)) {
            cerr << "Error: This vertex id already exists!!" << endl;
            return nullptr;
        } else if (isOutputQubit(qubit)) {
            cerr << "Error: This qubit's output already exists!!" << endl;
            return nullptr;
        } 
    }
    ZXVertex* v = new ZXVertex(id, qubit, VertexType::BOUNDARY);
    _vertices.push_back(v);
    _outputs.push_back(v);
    setOutputHash(qubit, v);
    if (verbose >= 5) cout << "Add output " << id << endl;
    return v;
    
}

ZXVertex* ZXGraph::addVertex(size_t id, int qubit, VertexType vt, Phase phase, bool checked) {
    if (!checked) {
        if (isId(id)) {
            cerr << "Error: This vertex id is already exist!!" << endl;
            return nullptr;
        } else if (vt == VertexType::BOUNDARY) {
            cerr << "Error: Use ADDInput / ADDOutput to add input vertex or output vertex!!" << endl;
            return nullptr;
        }
    }
    ZXVertex* v = new ZXVertex(id, qubit, vt, phase);
    _vertices.push_back(v);
    if (verbose >= 5) cout << "Add vertex " << id << endl;
    return v;
}
/**
 * @brief Add edge (<<vs, vt>, et>)
 *
 * @param vs
 * @param vt
 * @param et
 * @return EdgePair
 */
EdgePair ZXGraph::addEdge(ZXVertex* vs, ZXVertex* vt, EdgeType* et, bool allowSelfloop) {
    // if (vs->getType() == VertexType::BOUNDARY && vs->getNumNeighbors() >= 1) {
    //     cerr << "Boundary vertex " << vs->getId() << " must not have more than one neighbor." << endl;
    // } else if (vt->getType() == VertexType::BOUNDARY && vt->getNumNeighbors() >= 1) {
    //     cerr << "Boundary vertex " << vt->getId() << " must not have more than one neighbor." << endl;
    // } else if (vs == vt && vs->getType() == VertexType::BOUNDARY) {
    //     cerr << "Boundary vertex " << vs->getId() << " must not have self loop" << endl;
    // } else {
        // NeighborMap mode
    // if(vt == vs && (!allowSelfloop)){
    //     if (*et == EdgeType::HADAMARD){
    //         vt->setPhase(vt->getPhase() + Phase(1));
    //         if (verbose >= 5) cout << "Add phase Pi to " << vt->getId() << " due to hadamard selfloop" << endl;
    //     }
    //     else if (verbose >= 5) cout << "Neglect simple selfloop on " << vt->getId() << endl;
    //     return make_pair(make_pair(nullptr, nullptr), nullptr);
    // }
    // else{
        vs->addNeighbor(make_pair(vt, et));
        vt->addNeighbor(make_pair(vs, et));
        _edges.emplace_back(make_pair(vs, vt), et);
        if (verbose >= 5) cout << "Add edge ( " << vs->getId() << ", " << vt->getId() << " )" << endl;
        return _edges.back();
    // }
    
    // }

    // return make_pair(make_pair(nullptr, nullptr), nullptr);
}

void ZXGraph::addEdgeById(size_t id_s, size_t id_t, EdgeType* et) {
    if (!isId(id_s))
        cerr << "Error: id_s provided does not exist!" << endl;
    else if (!isId(id_t))
        cerr << "Error: id_t provided does not exist!" << endl;
    else {
        ZXVertex* vs = findVertexById(id_s);
        ZXVertex* vt = findVertexById(id_t);
        addEdge(vs, vt, et);
    }
}

void ZXGraph::addInputs(vector<ZXVertex*> inputs) {
    _inputs.insert(_inputs.end(), inputs.begin(), inputs.end());
}

void ZXGraph::addOutputs(vector<ZXVertex*> outputs) {
    _outputs.insert(_outputs.end(), outputs.begin(), outputs.end());
}

void ZXGraph::addVertices(vector<ZXVertex*> vertices) {
    _vertices.insert(_vertices.end(), vertices.begin(), vertices.end());
}

void ZXGraph::addEdges(vector<EdgePair> edges) {
    _edges.insert(_edges.end(), edges.begin(), edges.end());
}

void ZXGraph::mergeInputList(unordered_map<size_t, ZXVertex*> lst) {
    _inputList.merge(lst);
}

void ZXGraph::mergeOutputList(unordered_map<size_t, ZXVertex*> lst) {
    _outputList.merge(lst);
}

/**
 * @brief Remove `v` in ZXGraph and maintain the relationship between each vertex.
 *
 * @param v
 * @param checked
 */
void ZXGraph::removeVertex(ZXVertex* v, bool checked) {
    if (!checked) {
        if (!isId(v->getId())) {
            cerr << "This vertex does not exist!" << endl;
            return;
        }
    }

    if (verbose >= 5) cout << "Remove ID: " << v->getId() << endl;

    // Check if also in _inputs or _outputs
    if (auto itr = find(_inputs.begin(), _inputs.end(), v); itr != _inputs.end()) {
        _inputList.erase(v->getQubit());
        _inputs.erase(itr);
    }
    if (auto itr = find(_outputs.begin(), _outputs.end(), v); itr != _outputs.end()) {
        _outputList.erase(v->getQubit());
        _outputs.erase(itr);
    }

    // Check _edges
    vector<EdgePair> newEdges;
    for (size_t i = 0; i < _edges.size(); i++) {
        if (_edges[i].first.first == v || _edges[i].first.second == v)
            _edges[i].first.first->disconnect(_edges[i].first.second, true);
        else {
            newEdges.push_back(_edges[i]);
        }
    }
    setEdges(newEdges);

    // Check _vertices
    _vertices.erase(find(_vertices.begin(), _vertices.end(), v));

    // deallocate ZXVertex
    delete v;
}

/**
 * @brief Remove all vertex in vertices by calling `removeVertex(ZXVertex* v, bool checked)`
 *
 * @param vertices
 * @param checked
 */
void ZXGraph::removeVertices(vector<ZXVertex*> vertices, bool checked) {

    unordered_set<ZXVertex*> removing;
    for (const auto& v : vertices) {
        if (!checked) {
            if (!isId(v->getId())) {
                cerr << "Vertex " << v->getId() << " does not exist!" << endl;
                continue;
            }
        }
        if (verbose >= 5) cout << "Remove ID: " << v->getId() << endl;
        removing.insert(v);
    }

    // Check if also in _inputs or _outputs
    vector<ZXVertex*> newInputs;
    unordered_map<size_t, ZXVertex*> newInputList;
    for (const auto& v : _inputs) {
        if (!removing.contains(v)) {
            newInputs.push_back(v);
            newInputList[v->getQubit()] = v;
        }
    }
    setInputs(newInputs);
    setInputList(newInputList);

    vector<ZXVertex*> newOutputs;
    unordered_map<size_t, ZXVertex*> newOutputList;
    for (const auto& v : _outputs) {
        if (!removing.contains(v)) {
            newOutputs.push_back(v);
            newOutputList[v->getQubit()] = v;
        }
    }
    setOutputs(newOutputs);
    setOutputList(newOutputList);

    vector<EdgePair> newEdges;

    // Check _edges
    for (const auto& edge : _edges) {
        if (removing.contains(edge.first.first) || removing.contains(edge.first.second)) {
            edge.first.first->disconnect(edge.first.second, true);
        } else {
            newEdges.push_back(edge);
        }
    }
    setEdges(newEdges);
    // Check _vertices
    vector<ZXVertex*> newVertices;
    for (const auto& v : _vertices) {
        if (!removing.contains(v)) {
            newVertices.push_back(v);
        }
    }
    setVertices(newVertices);

    // deallocate ZXVertex
    for (const auto& v : removing) {
        delete v;
    }
}

/**
 * @brief Remove vertex by vertex's id. (Used in ZXCmd.cpp)
 *
 * @param id
 */
void ZXGraph::removeVertexById(const size_t& id) {
    auto v = findVertexById(id);
    if (v != nullptr)
        removeVertex(v, true);
    else
        cerr << "Error: This vertex id does not exist!!" << endl;
}

/**
 * @brief Remove all vertices with no neighbor.
 *
 */
void ZXGraph::removeIsolatedVertices() {
    vector<ZXVertex*> removing;
    for (const auto& v : _vertices) {
        if (v->getNeighborMap().empty()) {
            removing.push_back(v);
        }
    }
    removeVertices(removing);
}

/**
 * @brief Remove all edges between `vs` and `vt` by pointer.
 *
 * @param vs
 * @param vt
 * @param checked
 */
void ZXGraph::removeEdge(ZXVertex* vs, ZXVertex* vt, bool checked) {
    if (!checked) {
        if (!vs->isNeighbor(vt) || !vt->isNeighbor(vs)) {
            cerr << "Error: Vertex " << vs->getId() << " and " << vt->getId() << " are not connected!" << endl;
            return;
        }
    }
    for (size_t i = 0; i < _edges.size();) {
        if ((_edges[i].first.first == vs && _edges[i].first.second == vt) || (_edges[i].first.first == vt && _edges[i].first.second == vs)) {
            _edges.erase(_edges.begin() + i);
        } else
            i++;
    }
    vs->disconnect(vt, true);

    if (verbose >= 5) cout << "Remove edge ( " << vs->getId() << ", " << vt->getId() << " )" << endl;
}

/**
 * @brief Remove an edge exactly equal to `ep`.
 *
 * @param ep
 */
void ZXGraph::removeEdgeByEdgePair(const EdgePair& ep) {
    for (size_t i = 0; i < _edges.size(); i++) {
        if ((ep.first.first == _edges[i].first.first && ep.first.second == _edges[i].first.second && ep.second == _edges[i].second) || 
             (ep.first.first == _edges[i].first.second && ep.first.second == _edges[i].first.first && ep.second == _edges[i].second)) {
            if (verbose >= 5) cout << "Remove (" << ep.first.first->getId() << ", " << ep.first.second->getId() << " )" << endl;
            NeighborMap nb = ep.first.first->getNeighborMap();
            auto neighborItr = nb.equal_range(ep.first.second);
            for (auto itr = neighborItr.first; itr != neighborItr.second; ++itr) {
                if (itr->second == ep.second) {
                    nb.erase(itr);
                    ep.first.first->setNeighborMap(nb);
                    break;
                }
            }
            nb = ep.first.second->getNeighborMap();
            neighborItr = nb.equal_range(ep.first.first);
            for (auto itr = neighborItr.first; itr != neighborItr.second; ++itr) {
                if (itr->second == ep.second) {
                    nb.erase(itr);
                    ep.first.second->setNeighborMap(nb);
                    break;
                }
            }
            delete ep.second;
            _edges.erase(_edges.begin() + i);

            return;
        }
    }
}

void ZXGraph::removeEdgesByEdgePairs(const vector<EdgePair>& eps) {
    unordered_set<EdgePair> removing;
    for (const auto& ep : eps) {
        removing.insert(ep);
        if (verbose >= 5) {
            cout << "Remove (" << ep.first.first->getId() << ", " << ep.first.second->getId() << " )" << endl;
        }

        NeighborMap nbm = ep.first.first->getNeighborMap();
        auto result = nbm.equal_range(ep.first.second);
        for (auto& itr = result.first; itr != result.second; ++itr) {
            if (itr->second == ep.second) {
                nbm.erase(itr);
                ep.first.first->setNeighborMap(nbm);
                break;
            }
        }

        nbm = ep.first.second->getNeighborMap();
        result = nbm.equal_range(ep.first.first);
        for (auto& itr = result.first; itr != result.second; ++itr) {
            if (itr->second == ep.second) {
                nbm.erase(itr);
                ep.first.second->setNeighborMap(nbm);
                break;
            }
        }
    }

    vector<EdgePair> newEdges;

    for (const auto& edge : _edges) {
        if (!removing.contains(edge) && !removing.contains(
            make_pair(make_pair(edge.first.second, edge.first.first), edge.second)
        )) {
            newEdges.push_back(edge);
        }
    } 
    setEdges(newEdges);
}

/**
 * @brief Remove all edges between `vs` and `vt` by vertex's id.
 *
 * @param id_s
 * @param id_t
 */
void ZXGraph::removeEdgeById(const size_t& id_s, const size_t& id_t) {
    if (!isId(id_s))
        cerr << "Error: id_s provided does not exist!" << endl;
    else if (!isId(id_t))
        cerr << "Error: id_t provided does not exist!" << endl;
    else {
        ZXVertex* vs = findVertexById(id_s);
        ZXVertex* vt = findVertexById(id_t);
        removeEdge(vs, vt);
    }
}

/**
 * @brief adjoint the zxgraph
 * 
 */
void ZXGraph::adjoint() {
    swap(_inputs, _outputs);
    swap(_inputList, _outputList);
    for (auto& v : _vertices) v->setPhase(-v->getPhase());
}

/**
 * @brief Assign rotation/value to the specified boundary 
 *
 * @param qubit
 * @param isInput
 * @param ty
 * @param phase
 */
void ZXGraph::assignBoundary(size_t qubit, bool isInput, VertexType ty, Phase phase){
    ZXVertex* v = addVertex(findNextId(), qubit, ty, phase);
    ZXVertex* boundary = isInput ? _inputList[qubit] : _outputList[qubit];
    EdgeType e = *(boundary -> getNeighborMap().begin()->second);
    ZXVertex* nebBound = boundary->getNeighbor(0);
    removeVertex(boundary);
    // removeEdge(boundary, nebBound);
    addEdge(v, nebBound, new EdgeType(e));
}


// Find functions

/**
 * @brief Find Vertex by vertex's id.
 *
 * @param id
 * @return ZXVertex*
 */
ZXVertex* ZXGraph::findVertexById(const size_t& id) const {
    for (size_t i = 0; i < _vertices.size(); i++) {
        if (_vertices[i]->getId() == id) return _vertices[i];
    }
    return nullptr;
}

/**
 * @brief Find the next id that is never been used.
 *
 * @return size_t
 */
size_t ZXGraph::findNextId() const {
    size_t nextId = 0;
    for (size_t i = 0; i < _vertices.size(); i++) {
        if (_vertices[i]->getId() >= nextId) nextId = _vertices[i]->getId() + 1;
    }
    return nextId;
}

// Action
void ZXGraph::reset() {
    // for(size_t i = 0; i < _vertices.size(); i++) delete _vertices[i];
    // for(size_t i = 0; i < _topoOrder.size(); i++) delete _topoOrder[i];
    _inputs.clear();
    _outputs.clear();
    _vertices.clear();
    _edges.clear();
    _inputList.clear();
    _outputList.clear();
    _topoOrder.clear();
    _globalDFScounter = 1;
}

ZXGraph* ZXGraph::copy() const {
    //! Check if EdgeType change simultaneously
    
    ZXGraph* newGraph = new ZXGraph(0);
    unordered_map<size_t, ZXVertex*> id2vertex;
    newGraph->setId(getId());

    newGraph->_inputs.reserve(this->getNumInputs());
    newGraph->_inputList.reserve(this->getNumInputs());
    newGraph->_outputs.reserve(this->getNumOutputs());
    newGraph->_outputList.reserve(this->getNumOutputs());
    newGraph->_vertices.reserve(this->getNumVertices());
    id2vertex.reserve(this->getNumVertices());
    newGraph->_edges.reserve(this->getNumEdges());
    // new Inputs
    for (const auto& v : this->getInputs()) {
        id2vertex[v->getId()] = newGraph->addInput(v->getId(), v->getQubit(), true);
        
    }

    // new Outputs
    for (const auto& v : this->getOutputs()) {
        id2vertex[v->getId()] = newGraph->addOutput(v->getId(), v->getQubit(), true);
    }

    // new Vertices (without I/O)
    for (const auto& v : this->getVertices()) {
        if (v->getType() != VertexType::BOUNDARY) {
            id2vertex[v->getId()] = newGraph->addVertex(v->getId(), v->getQubit(), v->getType(), v->getPhase(), true);
        }
    }

    for (const auto& [vpair, etype]: this->getEdges()) {
        newGraph->addEdge(id2vertex[vpair.first->getId()], id2vertex[vpair.second->getId()], new EdgeType(*etype));
    }
    
    return newGraph;
}

void ZXGraph::sortIOByQubit() {
    sort(_inputs.begin(), _inputs.end(), [](ZXVertex* a, ZXVertex* b) { return a->getQubit() < b->getQubit(); });
    sort(_outputs.begin(), _outputs.end(), [](ZXVertex* a, ZXVertex* b) { return a->getQubit() < b->getQubit(); });
}

void ZXGraph::sortVerticeById() {
    sort(_vertices.begin(), _vertices.end(), [](ZXVertex* a, ZXVertex* b) { return a->getId() < b->getId(); });
}

void ZXGraph::liftQubit(const size_t& n) {
    for_each(_vertices.begin(), _vertices.end(), [&n](ZXVertex* v) { v->setQubit(v->getQubit() + n); });

    unordered_map<size_t, ZXVertex*> newInputList, newOutputList;

    for_each(_inputList.begin(), _inputList.end(),
             [&n, &newInputList](pair<size_t, ZXVertex*> itr) {
                 newInputList[itr.first + n] = itr.second;
             });
    for_each(_outputList.begin(), _outputList.end(),
             [&n, &newOutputList](pair<size_t, ZXVertex*> itr) {
                 newOutputList[itr.first + n] = itr.second;
             });

    setInputList(newInputList);
    setOutputList(newOutputList);
}

// Print functions
void ZXGraph::printGraph() const {
    cout << "Graph " << _id << endl;
    cout << setw(15) << left << "Inputs: " << _inputs.size() << endl;
    cout << setw(15) << left << "Outputs: " << _outputs.size() << endl;
    cout << setw(15) << left << "Vertices: " << _vertices.size() << endl;
    cout << setw(15) << left << "Edges: " << _edges.size() << endl;
}

void ZXGraph::printInputs() const {
    for (size_t i = 0; i < _inputs.size(); i++) {
        cout << "Input " << i + 1 << setw(8) << left << ":" << _inputs[i]->getId() << endl;
    }
    cout << "Total #Inputs: " << _inputs.size() << endl;
}

void ZXGraph::printOutputs() const {
    for (size_t i = 0; i < _outputs.size(); i++) {
        cout << "Output " << i + 1 << setw(7) << left << ":" << _outputs[i]->getId() << endl;
    }
    cout << "Total #Outputs: " << _outputs.size() << endl;
}

void ZXGraph::printVertices() const {
    cout << "\n";
    for (size_t i = 0; i < _vertices.size(); i++) {
        _vertices[i]->printVertex();
    }
    cout << "Total #Vertices: " << _vertices.size() << endl;
    cout << "\n";
}

void ZXGraph::printEdges() const {
    for (size_t i = 0; i < _edges.size(); i++) {
        cout << "( " << _edges[i].first.first->getId() << ", " << _edges[i].first.second->getId() << " )\tType:\t" << EdgeType2Str(_edges[i].second) << endl;
    }
    cout << "Total #Edges: " << _edges.size() << endl;
}

void ZXGraph::printEdge(size_t idx) const{
    if(idx < _edges.size())
        cout << "( " << _edges[idx].first.first->getId() << ", " << _edges[idx].first.second->getId() << " )\tType:\t" << EdgeType2Str(_edges[idx].second) << endl;
}

EdgePair makeEdgeKey(ZXVertex* v1, ZXVertex* v2, EdgeType* et) {
    return make_pair(
        (v2->getId() < v1->getId()) ? make_pair(v2, v1) : make_pair(v1, v2), et);
}

EdgePair makeEdgeKey(EdgePair epair) {
    return make_pair(
        (epair.first.second->getId() < epair.first.first->getId()) ? make_pair(epair.first.second, epair.first.first) : make_pair(epair.first.first, epair.first.second), epair.second);
}

EdgeKey makeEdgeKey(ZXVertex* v1, ZXVertex* v2, EdgeType et) {
    return make_pair(
        (v2->getId() < v1->getId()) ? make_pair(v2, v1) : make_pair(v1, v2), et);
}

EdgeKey makeEdgeKey(EdgeKey epair) {
    return make_pair(
        (epair.first.second->getId() < epair.first.first->getId()) ? make_pair(epair.first.second, epair.first.first) : make_pair(epair.first.first, epair.first.second), epair.second);
}