/****************************************************************************
  FileName     [ zxMapping.cpp ]
  PackageName  [ graph ]
  Synopsis     [ Mapper class for ZX-to-Tensor mapping ]
  Author       [ Chin-Yi Cheng, Mu-Te Joshua Lau ]
  Copyright    [ Copyleft(c) 2022-present DVLab, GIEE, NTU, Taiwan ]
****************************************************************************/
#include "zx2tsMapper.h"
extern size_t verbose;

using namespace std;

// map a ZX-diagram to a tensor
void ZX2TSMapper::map() {
    if (verbose >= 3) cout << "---- TRAVERSE AND BUILD THE TENSOR ----" << endl;
    _zxgraph->topoTraverse([this](ZXVertex* v) { mapOneVertex(v); });
    for (size_t i = 0; i < _boundaryEdges.size(); i++)
        _tensorList.frontiers(i).emplace(_boundaryEdges[i], 0);
    // cout << _tensorList.tensor(0) << endl;
    TensorAxisList inputIds, outputIds;
    getAxisOrders(inputIds, _zxgraph->getInputList());
    getAxisOrders(outputIds, _zxgraph->getOutputList());

    _tensorList.tensor(0) = _tensorList.tensor(0).transpose(concatAxisList(inputIds, outputIds));
    if (verbose >= 3) {
        cout << "\nThe resulting tensor is: \n"<< _tensorList.tensor(0) << endl;
    }
}

// map one vertex
void ZX2TSMapper::mapOneVertex(ZXVertex* v) {
    _normalPin.clear();
    _hadamardPin.clear();
    _removeEdge.clear();
    _addEdge.clear();
    _tensorId = 0;

    if (verbose >= 3) cout << "> Mapping vertex " << v->getId() << " (" << VertexType2Str(v->getType()) << "): ";
    if (isOfNewGraph(v)) {
        if (verbose >= 3) cout << "New Subgraph" << endl;
        initSubgraph(v);
    } else if (v->getType() == VertexType::BOUNDARY) {
        if (verbose >= 3) cout << "Boundary Node" << endl;
    } else {
        if (verbose >= 3) cout << "Tensordot" << endl;
        updatePinsAndFrontiers(v);
        // if (verbose >= 7) printFrontiers();
        tensorDotVertex(v);
    }
    v->setPin(_tensorId);
    if (verbose >= 7){
        printFrontiers();
    }
}

// Generate a new subgraph for mapping
void ZX2TSMapper::initSubgraph(ZXVertex* v) {
    NeighborMap neighborMap = v->getNeighborMap();

    _tensorList.append(Frontiers(), QTensor<double>(1. + 0.i));
    _tensorId = _tensorList.size() - 1;
    assert(v->getType() == VertexType::BOUNDARY);
    EdgeKey edgeKey = makeEdgeKey(v, neighborMap.begin()->first, *(neighborMap.begin()->second));
    currTensor() = tensordot(currTensor(), QTensor<double>::identity(neighborMap.size()));
    _boundaryEdges.push_back(edgeKey);
    currFrontiers().emplace(edgeKey, 1);

    return;
}

// Check if a vertex belongs to a new subgraph that is not traversed
bool ZX2TSMapper::isOfNewGraph(const ZXVertex* v) {
    for (auto epair : v->getNeighborMap()) {
        if (isFrontier(epair)) {
            _tensorId = epair.first->getPin();
            return false;
        }
    }
    return true;
}

// Print the current and next frontiers
void ZX2TSMapper::printFrontiers() const {
    // cout << "**************" << endl;
    // cout << "Current Frontier Edges: " << endl;
    // for (size_t i = 0; i < _removeEdge.size(); i++) {
    //     cout << _removeEdge[i].first.first->getId() << "--" << _removeEdge[i].first.second->getId() << endl;
    // }
    // cout << "Next Frontier Edges: " << endl;
    // for (size_t i = 0; i < _addEdge.size(); i++) {
    //     cout << _addEdge[i].first.first->getId() << "--" << _addEdge[i].first.second->getId() << endl;
    // }
    // cout << "**************" << endl;
    cout << "  - Current frontiers: " << endl;
    for(auto i=currFrontiers().begin(); i!=currFrontiers().end(); i++){
        cout << "    " << i->first.first.first->getId() << "--" << i->first.first.second->getId() << " (" << EdgeType2Str(&(i->first.second)) << ") pin id: " << i->second << endl;
    }
}

// Check if a pair<ZXVertex*, EdgeType*> is a frontier to some subgraph
bool ZX2TSMapper::isFrontier(const pair<ZXVertex*, EdgeType*>& nbr) const {
    return (nbr.first->getPin() != unsigned(-1));
}

// Create an EdgePair that is used as the key to the Frontiers
// EdgePair ZX2TSMapper::makeEdgeKey(ZXVertex* v1, ZXVertex* v2, EdgeType* et) {
//     return make_pair(
//         (v2->getId() < v1->getId()) ? make_pair(v2, v1) : make_pair(v1, v2),
//         et);
// }

// Get the order of inputs and outputs
void ZX2TSMapper::getAxisOrders(TensorAxisList& axList, const std::unordered_map<size_t, ZXVertex*>& ioList) {
    axList.resize(ioList.size());
    for (auto& [qubitId, vertex] : ioList) {
        // cout << qubitId << ", " << vertex->getId() << endl;
        NeighborMap nebs = vertex->getNeighborMap();
        auto& [neighbor, etype] = *(nebs.begin());
        EdgeKey edgeKey = makeEdgeKey(vertex, neighbor, *etype);
        // cout << vertex->getId() << ", "<<  neighbor->getId() << endl;
        axList[qubitId] = _tensorList.frontiers(0).find(edgeKey)->second;
        // cout << axList[qubitId] << endl;
    }
}

// update information for the current and next frontiers
void ZX2TSMapper::updatePinsAndFrontiers(ZXVertex* v) {
    NeighborMap neighborMap = v->getNeighborMap();
    NeighborMap frontAlreadyRetrived;
    vector<pair<ZXVertex *, EdgeType >> tmp;
    for (auto epair : neighborMap) {
        ZXVertex* const& neighbor = epair.first;
        EdgeType* const& etype = epair.second;
        if (v == neighbor) { // omit self loops
            if (verbose >= 7) cout << "  - Skipping self loop: " << v->getId() << "--" << neighbor->getId() << " (" <<  EdgeType2Str(etype) << ")" << endl;
            continue;
        } 
        EdgeKey edgeKey = makeEdgeKey(v, neighbor, *etype);
        if (isFrontier(epair)) {
            bool newSeen = find(tmp.begin(),tmp.end(),make_pair(neighbor,*etype))==tmp.end();
            tmp.push_back(make_pair(neighbor,*etype));
            if(newSeen){
                auto result = currFrontiers().equal_range(edgeKey);
                for (auto jtr = result.first; jtr != result.second; jtr++) {
                    auto& [epair, id] = *jtr;
                    // cout << jtr->first.first.first->getId() << "--" << jtr->first.first.second->getId() << " (" << jtr->first.second << ") pin id: " << jtr->second << endl;
                    if ((epair.second) == EdgeType::HADAMARD)
                        _hadamardPin.push_back(id);
                    else
                        _normalPin.push_back(id);
                    _removeEdge.push_back(edgeKey);
                }
            }
            
        } else
            _addEdge.push_back(edgeKey);
    }
}

// Convert hadamard edges to normal edges and returns a corresponding tensor
QTensor<double> ZX2TSMapper::dehadamardize(const QTensor<double>& ts) {
    QTensor<double> HTensorProduct = tensorPow(
        QTensor<double>::hbox(2), _hadamardPin.size());

    TensorAxisList connect_pin;
    for (size_t t = 0; t < _hadamardPin.size(); t++)
        connect_pin.push_back(2 * t);
    QTensor<double> tmp = tensordot(ts, HTensorProduct, _hadamardPin, connect_pin);
    // All edges shoud be updated here
    for(auto it=currFrontiers().begin();it!=currFrontiers().end();it++){
        if(find(_normalPin.begin(),_normalPin.end(),it->second)==_normalPin.end()){
            if(find(_hadamardPin.begin(),_hadamardPin.end(),it->second)==_hadamardPin.end()){
               it->second = tmp.getNewAxisId(it->second); 
            }    
        }    
    }
    ////////////////////////////////////
    for (size_t t = 0; t < _hadamardPin.size(); t++)
        _hadamardPin[t] = tmp.getNewAxisId(ts.dimension() + connect_pin[t] + 1);  // dimension of big tensor + 1,3,5,7,9  
    // Normal Edges also
    for(size_t t=0; t<_normalPin.size(); t++)
        _normalPin[t] = tmp.getNewAxisId(_normalPin[t]);
    _normalPin = concatAxisList(_hadamardPin, _normalPin);
    
    return tmp;
}

// tensordot the current tensor to the vertex's tensor form
void ZX2TSMapper::tensorDotVertex(ZXVertex* v) {
    
    QTensor<double> dehadamarded = dehadamardize(currTensor());
    TensorAxisList connect_pin;
    for (size_t t = 0; t < _normalPin.size(); t++)
        connect_pin.push_back(t);

    currTensor() = tensordot(dehadamarded, v->getTSform(), _normalPin, connect_pin);
    
    // 3. update pins
    for (size_t i = 0; i < _removeEdge.size(); i++){
        currFrontiers().erase(_removeEdge[i]);  // Erase old edges
    }
    for (auto& frontier : currFrontiers()) {
        frontier.second = currTensor().getNewAxisId(frontier.second);
    }

    connect_pin.clear();
    for (size_t t = 0; t < _addEdge.size(); t++)
        connect_pin.push_back(_normalPin.size() + t);

    
    for (size_t t = 0; t < _addEdge.size(); t++) {
        size_t newId = currTensor().getNewAxisId(dehadamarded.dimension() + connect_pin[t]);
        currFrontiers().emplace(_addEdge[t], newId);  // origin pin (neighbot count) + 1,3,5,7,9
    }
}
