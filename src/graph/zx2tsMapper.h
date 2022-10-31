/****************************************************************************
  FileName     [ zxMapping.cpp ]
  PackageName  [ graph ]
  Synopsis     [ Mapper class for ZX-to-Tensor mapping ]
  Author       [ Chin-Yi Cheng, Mu-Te Joshua Lau ]
  Copyright    [ Copyleft(c) 2022-present DVLab, GIEE, NTU, Taiwan ]
****************************************************************************/
#ifndef ZX2TS_MAPPER_H
#define ZX2TS_MAPPER_H
#include <iostream>
#include <vector>
#include <cassert>
#include <iomanip>
#include <algorithm>
#include "zxGraph.h"
#include "util.h"

class ZX2TSMapper {
public:
    using Frontiers = unordered_multimap<EdgeKey,size_t>;

    ZX2TSMapper(ZXGraph* zxg): _zxgraph(zxg) {}
    class ZX2TSList {
    public:
        const Frontiers& frontiers(const size_t& id) const {
            return _zx2tsList[id].first;
        }
        const QTensor<double>& tensor(const size_t& id) const {
            return _zx2tsList[id].second;
        }
        Frontiers& frontiers(const size_t& id) {
            return _zx2tsList[id].first;
        }
        QTensor<double>& tensor(const size_t& id) {
            return _zx2tsList[id].second;
        }
        void append(const Frontiers& f, const QTensor<double>& q) {
            _zx2tsList.emplace_back(f, q);
        }
        size_t size() {
            return _zx2tsList.size();
        }
    private:
        vector<pair<Frontiers, QTensor<double>>> _zx2tsList; 
    };
    
    bool map();

private:
    ZXGraph*            _zxgraph;       // The ZX Graph to be mapped
    vector<EdgeKey>     _boundaryEdges; // EdgePairs of the boundaries 
    ZX2TSList           _zx2tsList;     // The tensor list for each set of frontiers
    size_t              _tensorId;      // Current tensor id for the _tensorId

    TensorAxisList      _simplePins;     // Axes that can be tensordotted directly
    TensorAxisList      _hadamardPins;   // Axes that should be applied hadamards first
    vector<EdgeKey>     _removeEdges;    // Old frontiers to be removed
    vector<EdgeKey>     _addEdges;       // New frontiers to be added
    
    Frontiers&             currFrontiers()       { return _zx2tsList.frontiers(_tensorId); }
    QTensor<double>&       currTensor()          { return _zx2tsList.tensor(_tensorId); }
    const Frontiers&       currFrontiers() const { return _zx2tsList.frontiers(_tensorId); }
    const QTensor<double>& currTensor()    const { return _zx2tsList.tensor(_tensorId); }

    void mapOneVertex(ZXVertex* v);

    // mapOneVertex Subroutines
    void initSubgraph(ZXVertex* v);
    void updatePinsAndFrontiers(ZXVertex* v);
    QTensor<double> dehadamardize(const QTensor<double>& ts);
    void tensorDotVertex(ZXVertex* v);

    bool isOfNewGraph(const ZXVertex* v);
    bool isFrontier(const pair<ZXVertex*, EdgeType*>& nbr) const { return (nbr.first->getPin() != unsigned(-1)); }

    void printFrontiers() const;
    // EdgePair makeEdgeKey(ZXVertex* v1, ZXVertex* v2, EdgeType* et);
    void getAxisOrders(TensorAxisList& axList, const std::unordered_map<size_t, ZXVertex *>& ioList, bool isOutput = false);
};
#endif //ZX2TS_MAPPER_H