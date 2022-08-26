/****************************************************************************
  FileName     [ zxGraph.h ]
  PackageName  [ graph ]
  Synopsis     [ Define class ZXGraph member functions ]
  Author       [ Cheng-Hua Lu ]
  Copyright    [ Copyleft(c) 2022-present DVLab, GIEE, NTU, Taiwan ]
****************************************************************************/

#include <iostream>
#include <vector>
#include <cassert>
#include <iomanip>
#include <algorithm>
#include "zxGraph.h"
#include "util.h"

using namespace std;

/**************************************/
/*   class VT member functions   */
/**************************************/

// Add and Remove

void VT::removeNeighbor(pair<VT*, EdgeType> neighbor){
    if(find(_neighbors.begin(), _neighbors.end(), neighbor) != _neighbors.end()){
        cout << "  * remove " << neighbor.first->getId() << " from " << _id << endl;
        _neighbors.erase(find(_neighbors.begin(), _neighbors.end(), neighbor));
    } 
    else cerr << "Vertex " << neighbor.first->getId() << "is not a neighbor of " << _id << endl;
}


// Print functions

void VT::printVertex() const{
    cout << "ID:\t" << _id << "\t";
    cout << "VertexType:\t" << _type << "\t";
    cout << "Qubit:\t" << _qubit << "\t";
    cout << "#Neighbors:\t" << _neighbors.size() << "\t";
    printNeighbors();
    // cout << endl;;
}

void VT::printNeighbors() const{
    for(size_t i = 0; i < _neighbors.size(); i++){
        cout << "( " << _neighbors[i].first->getId() << ", " << _neighbors[i].second << " )  " ;
    }
    cout << endl;
}


// Test
bool VT::isNeighbor(VT* v) const{
    for(size_t i = 0; i < _neighbors.size(); i++){
        if(_neighbors[i].first == v) return true;
    }
    return false;
}


/**************************************/
/*   class ZXGraph member functions   */
/**************************************/

// For testing
void ZXGraph::generateCNOT(){
    cout << "Generate a 2-qubit CNOT graph for testing" << endl;
    // Generate Inputs
    vector<VT*> inputs, outputs, vertices;
    for(size_t i = 0; i < 2; i++){
        VT* input = new VT(vertices.size(), i, i, VertexType::BOUNDARY);
        inputs.push_back(input);
        vertices.push_back(input);
    }

    // Generate CNOT
    VT* z_spider = new VT(vertices.size(), 0, 0, VertexType::Z);
    vertices.push_back(z_spider);
    VT* x_spider = new VT(vertices.size(), 1, 1, VertexType::X);
    vertices.push_back(x_spider);

    // Generate Outputs
    for(size_t i = 0; i < 2; i++){
        VT* output = new VT(vertices.size(), i, i, VertexType::BOUNDARY);
        outputs.push_back(output);
        vertices.push_back(output);
    }
    setInputs(inputs);
    setOutputs(outputs);
    setVertices(vertices);

    // Generate edges [(0,2), (1,3), (2,3), (2,4), (3,5)]
    vector<pair<size_t, size_t> > edgeList;
    edgeList.push_back(make_pair(0,2));
    edgeList.push_back(make_pair(1,3));
    edgeList.push_back(make_pair(2,3));
    edgeList.push_back(make_pair(2,4));
    edgeList.push_back(make_pair(3,5));

    vector<pair<pair<VT*, VT*>, EdgeType> > edges;

    for(size_t i = 0; i < edgeList.size(); i++){
        if(findVertexById(edgeList[i].first) != nullptr && findVertexById(edgeList[i].second) != nullptr){
            VT* s = findVertexById(edgeList[i].first); VT* t = findVertexById(edgeList[i].second);
            s->addNeighbor(make_pair(t, EdgeType::SIMPLE));
            t->addNeighbor(make_pair(s, EdgeType::SIMPLE));
            edges.push_back(make_pair(make_pair(s, t), EdgeType::SIMPLE));
        }
    }
    setEdges(edges);
    setQubitCount(_inputs.size());
}

bool ZXGraph::isEmpty() const{
    if(_inputs.empty() && _outputs.empty() && _vertices.empty() && _edges.empty() && _nqubit == 0) return true;
    return false;
}

bool ZXGraph::isValid() const{
    for(size_t i = 0; i < _edges.size(); i++){
        if(!_edges[i].first.first->isNeighbor(_edges[i].first.second) || 
           !_edges[i].first.second->isNeighbor(_edges[i].first.first))  return false;
    }
    return true;
}

bool ZXGraph::isConnected(VT* v1, VT* v2) const{
    if(v1->isNeighbor(v2) && v2->isNeighbor(v1)) return true;
    return false;
}

bool ZXGraph::isId(size_t id) const{
    for(size_t i = 0; i < _vertices.size(); i++){
        if(_vertices[i]->getId() == id) return true;
    }
    return false;
}


// Add and Remove
void ZXGraph::addInput(size_t id, int qubit, VertexType vt){
    if(isId(id)) cerr << "Error: This vertex id is already exist!!" << endl;
    else{
        VT* v = new VT(id, qubit, vt);
        _vertices.push_back(v);
        _inputs.push_back(v);
    }
}

void ZXGraph::addOutput(size_t id, int qubit, VertexType vt){
    if(isId(id)) cerr << "Error: This vertex id is already exist!!" << endl;
    else{
        VT* v = new VT(id, qubit, vt);
        _vertices.push_back(v);
        _outputs.push_back(v);
    }
}

void ZXGraph::addVertex(size_t id, int qubit, VertexType vt){
    if(isId(id)) cerr << "Error: This vertex id is already exist!!" << endl;
    else if(vt == VertexType::BOUNDARY) cerr << "Error: Use ADDInput / ADDOutput to add input vertex or output vertex!!" << endl;
    else{
        VT* v = new VT(id, qubit, vt);
        _vertices.push_back(v);
    }
}

void ZXGraph::removeVertex(VT* v){
    if(!isId(v->getId())) cerr << "This vertex is not exist!" << endl;
    else{
        cout << "Remove ID: " << v->getId() << endl;
        // Remove all neighbors' records
        for(size_t i = 0; i < v->getNeighbors().size(); i++){
            v->getNeighbors()[i].first->removeNeighbor(make_pair(v, v->getNeighbors()[i].second));
        }
        // Check if also in _inputs or _outputs
        if(find(_inputs.begin(), _inputs.end(), v) != _inputs.end()) _inputs.erase(find(_inputs.begin(), _inputs.end(), v));
        if(find(_outputs.begin(), _outputs.end(), v) != _outputs.end()) _outputs.erase(find(_outputs.begin(), _outputs.end(), v));

        // Check _edges
        for(size_t i = 0; i < _edges.size();){
            if(_edges[i].first.first == v || _edges[i].first.second == v) _edges.erase(_edges.begin()+i);
            else i++;
        }

        // Check _vertices
        _vertices.erase(find(_vertices.begin(), _vertices.end(), v));

        // deallocate VT
        delete v;
    }
}

void ZXGraph::removeVertexById(size_t id){
    if(findVertexById(id) != nullptr) removeVertex(findVertexById(id));
    else cerr << "Error: This vertex id is not exist!!" << endl;
}

void ZXGraph::removeIsolatedVertices(){
    for(size_t i = 0; i < _vertices.size(); ){
        if(_vertices[i]->getNeighbors().empty()){
            removeVertex(_vertices[i]);
        }
        else i++;
    }
}



// Find functions
VT* ZXGraph::findVertexById(size_t id) const{
    for(size_t i = 0; i < _vertices.size(); i++){
        if(_vertices[i]->getId() == id) return _vertices[i];
    }
    return nullptr;
}


// Print functions

void ZXGraph::printGraph() const{
    cout << "Graph " << _id << endl;
    cout << setw(15) << left << "Inputs: " << _inputs.size() << endl;
    cout << setw(15) << left << "Outputs: " << _outputs.size() << endl;
    cout << setw(15) << left << "Vertices: " << _vertices.size() << endl;
    cout << setw(15) << left << "Edges: " << _edges.size() << endl;
}

void ZXGraph::printInputs() const{
    for(size_t i = 0; i < _inputs.size(); i++){
        cout << "Input " << i+1 << setw(8) << left << ":" << _inputs[i]->getId() << endl;
    }
    cout << "Total #Inputs: " << _inputs.size() << endl;
}

void ZXGraph::printOutputs() const{
    for(size_t i = 0; i < _outputs.size(); i++){
        cout << "Output " << i+1 << setw(7) << left << ":" << _outputs[i]->getId() << endl;
    }
    cout << "Total #Outputs: " << _outputs.size() << endl;
}

void ZXGraph::printVertices() const{
    for(size_t i = 0; i < _vertices.size(); i++){
        _vertices[i]->printVertex();
    }
    cout << "Total #Vertices: " << _vertices.size() << endl;
}

void ZXGraph::printEdges() const{
    for(size_t i = 0; i < _edges.size(); i++){
        cout << "( " << _edges[i].first.first->getId() << ", " <<  _edges[i].first.second->getId() << " )\tType:\t" << _edges[i].second << endl; 
    }
    cout << "Total #Edges: " << _edges.size() << endl;
}


