/****************************************************************************
  FileName     [ zxGraph.h ]
  PackageName  [ graph ]
  Synopsis     [ Define ZX-graph structures ]
  Author       [ Cheng-Hua Lu, Chin-Yi Cheng ]
  Copyright    [ Copyleft(c) 2022-present DVLab, GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef ZX_GRAPH_H
#define ZX_GRAPH_H

#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>
#include "phase.h"
#include "qtensor.h"
#include "zxDef.h"
#include <iterator>
#include "ordered_hashset.h"
using namespace std;

class ZXVertex{
    public:
        class EdgeIterator;
        friend class EdgeIterator;

        ZXVertex(size_t id, int qubit, VertexType vt, Phase phase = Phase()) {
            _id = id;
            _qubit = qubit;
            _type = vt;
            _phase = phase;
            _DFSCounter = 0;
            _pin = unsigned(-1);
            _neighbors.clear();
        }
        ~ZXVertex(){}

        // Getter and Setter
        const size_t& getId() const                                         { return _id; }
        const int& getQubit() const                                         { return _qubit; }
        const VertexType& getType() const                                   { return _type; }
        const Phase& getPhase() const                                       { return _phase; }
        const size_t& getPin() const                                        { return _pin; }   
        
        const Neighbors& getNeighbors() const                                    { return _neighbors; }
        size_t getNumNeighbors() const                                           { return _neighbors.size(); }
        // QTensor<double> getTSform();

        void setId(const size_t& id)                                        { _id = id; }
        void setQubit(const int& q)                                         { _qubit = q; }
        void setType(const VertexType& ZXVertex)                            { _type = ZXVertex; }
        void setPhase(const Phase& p)                                       { _phase = p; }
        void setPin(const size_t& p)                                        { _pin = p; }

        // Add and Remove
        void addNeighbor(const NeighborPair& nb)                            { _neighbors.insert(nb); }
        void removeNeighbor(const NeighborPair& nb)                         { _neighbors.erase(nb); }


        // Print functions
        void printVertex() const;
        void printNeighbors() const;

        
        // Action
        void disconnect(ZXVertex* v, bool checked = false);

        // Test
        bool isNeighbor(ZXVertex* v) const;
        bool isNeighbor(const NeighborPair& nb) const { return _neighbors.contains(nb); }
        bool isZ()        const { return getType() == VertexType::Z; }
        bool isX()        const { return getType() == VertexType::X; }
        bool isHBox()     const { return getType() == VertexType::H_BOX; }
        bool isBoundary() const { return getType() == VertexType::BOUNDARY; }
        
        // DFS
        bool isVisited(unsigned global) { return global == _DFSCounter; }
        void setVisited(unsigned global) { _DFSCounter = global; }

    private:
        int                                  _qubit;
        size_t                               _id;
        VertexType                           _type;
        Phase                                _phase;
        Neighbors                            _neighbors;
        unsigned                             _DFSCounter;
        size_t                               _pin;
        
};

// using ZXVertexList  = unordered_set<ZXVertex*>;


class ZXGraph{
    public:
        ZXGraph(size_t id, void** ref = NULL) : _id(id), _ref(ref), _currentVertexId(0), _tensor(1.+0.i){
            _globalDFScounter = 1;
        }
        
        ~ZXGraph() {
            // for(size_t i = 0; i < _vertices.size(); i++) delete _vertices[i];
            for(const auto& ver: _vertices.range()) delete ver;
        }


        // Getter and Setter
        void setId(size_t id)                                           { _id = id; }
        void setRef(void** ref)                                         { _ref = ref; }
        
        void setInputs(const ZXVertexList& inputs)                      { _inputs = inputs; }
        void setOutputs(const ZXVertexList& outputs)                    { _outputs = outputs; }
        void setVertices(const ZXVertexList& vertices)                  { _vertices = vertices; }
        
        const size_t& getId() const                                     { return _id; }
        void** getRef() const                                           { return _ref; }
        const ZXVertexList& getInputs() const                           { return _inputs; }
        const ZXVertexList& getOutputs() const                          { return _outputs; }
        const ZXVertexList& getVertices() const                         { return _vertices; }
        size_t getNumInputs() const                                     { return _inputs.size(); }
        size_t getNumOutputs() const                                    { return _outputs.size(); }
        size_t getNumVertices() const                                   { return _vertices.size(); }
        size_t getNumEdges() const;

        //REVIEW - add: new function
        vector<ZXVertex*> getSortedListFromSet(const ZXVertexList& set) const;

        // For testing
        void generateCNOT();
        bool isEmpty() const;
        bool isValid() const;
        bool isConnected(ZXVertex* v1, ZXVertex* v2) const;
        bool isId(size_t id) const;
        bool isGraphLike() const;
        bool isInputQubit(int qubit) const;
        bool isOutputQubit(int qubit) const;


        // Add and Remove
        ZXVertex* addInput(int qubit, bool checked = false);
        ZXVertex* addOutput(int qubit, bool checked = false);
        ZXVertex* addVertex(int qubit, VertexType ZXVertex, Phase phase = Phase(), bool checked = false);

        void addInputs(const ZXVertexList& inputs);
        void addOutputs(const ZXVertexList& outputs);
        void addVertices(const ZXVertexList& vertices, bool reordered = false);
        EdgePair addEdge(ZXVertex* vs, ZXVertex* vt, EdgeType et);
        void addEdgeById(size_t id_s, size_t id_t, EdgeType et);
        
        void mergeInputList(unordered_map<size_t, ZXVertex*> lst);
        void mergeOutputList(unordered_map<size_t, ZXVertex*> lst);

        void removeVertex(ZXVertex* v, bool checked = false);

        void removeVertices(vector<ZXVertex* > vertices, bool checked = false);
        void removeVertexById(const size_t& id);

        void removeIsolatedVertices();

        void removeAllEdgesBetween(ZXVertex* vs, ZXVertex* vt, bool checked = false);
        void removeEdge(const EdgePair& ep);
        void removeEdges(const vector<EdgePair>& eps);
        void removeEdgeById(const size_t& id_s, const size_t& id_t, EdgeType etype = EdgeType::ERRORTYPE);

        // Operation on graph
        void adjoint();
        void assignBoundary(size_t qubit, bool input, VertexType type, Phase phase);

                
        // Find functions
        ZXVertex* findVertexById(const size_t& id) const;

        size_t findNextId() const;


        // Action
        void reset();
        ZXGraph* copy() const;
        void sortIOByQubit();
        void sortVerticeById();
        void liftQubit(const size_t& n);


        // Print functions
        //REVIEW provides filters?
        void printGraph() const;
        void printInputs() const;
        void printOutputs() const;
        void printVertices() const;
        void printEdges() const;

        
        // Traverse
        // void updateTopoOrder();
        // template<typename F>
        // void topoTraverse(F lambda){
        //     updateTopoOrder();
        //     for_each(_topoOrder.begin(),_topoOrder.end(),lambda);
        // }
        template<typename F>
        void forEachEdge(F lambda) const {
            for (auto v : _vertices.range()) {
                for (auto [nb, etype] : v->getNeighbors().range()) {
                    lambda(makeEdgePair(v, nb, etype));
                }
            }
        }

        // For mapping
        void tensorMapping();
        void concatenate(ZXGraph* tmp, bool remove_imm = false);
        void setInputHash(const size_t& q, ZXVertex* v)                    { _inputList[q] = v; }
        void setOutputHash(const size_t& q, ZXVertex* v)                   { _outputList[q] = v; }
        void setInputList(const unordered_map<size_t, ZXVertex*>& lst)     { _inputList = lst; }
        void setOutputList(const unordered_map<size_t, ZXVertex*>& lst)    { _outputList = lst; }
        const unordered_map<size_t, ZXVertex*>& getInputList() const       { return _inputList; }
        const unordered_map<size_t, ZXVertex*>& getOutputList() const      { return _outputList; }
        ZXVertex* getInputFromHash(const size_t& q);
        ZXVertex* getOutputFromHash(const size_t& q);
        ZXVertexList getNonBoundary();

        // I/O
        //REVIEW - overhauled: add complete mode (print all neighbors) to write
        bool readZX(string, bool bzx = false);
        bool writeZX(string, bool complete = false, bool bzx = false);

    private:
        size_t                            _id;
        void**                            _ref;
        size_t                            _currentVertexId; // keep the index
        QTensor<double>                   _tensor;
        vector<ZXVertex*>                 _inputs_depr;
        vector<ZXVertex*>                 _outputs_depr;
        vector<ZXVertex*>                 _vertices_depr;
        vector<EdgePair_depr >            _edges_depr;
        unordered_map<size_t, ZXVertex*>  _inputList;
        unordered_map<size_t, ZXVertex*>  _outputList;
        vector<ZXVertex*>                 _topoOrder;
        unsigned                          _globalDFScounter;
        ZXVertexList                      _vertices;
        ZXVertexList                      _inputs;
        ZXVertexList                      _outputs;
        // void DFS(ZXVertex*);

};

VertexType  str2VertexType(const string& str);
string      VertexType2Str(const VertexType& vt);
EdgeType    str2EdgeType(const string& str);
string      EdgeType2Str(const EdgeType& et);
EdgeType    toggleEdge(const EdgeType& et);


EdgeType*   str2EdgeType_depr(const string& str);
string      EdgeType2Str_depr(const EdgeType* et);

template <typename T>
ostream& operator<<(typename enable_if<is_enum<T>::value, ostream>::type& stream, const T& e) {
    return stream << static_cast<typename underlying_type<T>::type>(e);
}


EdgePair makeEdgePair(ZXVertex* v1, ZXVertex* v2, EdgeType et);
EdgePair makeEdgePair(EdgePair epair);
EdgePair makeEdgePairDummy();

EdgePair_depr makeEdgeKey_depr(ZXVertex* v1, ZXVertex* v2, EdgeType* et);
EdgePair_depr makeEdgeKey_depr(EdgePair_depr epair);

#endif