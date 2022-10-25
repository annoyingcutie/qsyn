/****************************************************************************
  FileName     [ tensorMgr.cpp ]
  PackageName  [ tensor ]
  Synopsis     [ Define tensor manager ]
  Author       [ Mu-Te (Joshua) Lau ]
  Copyright    [ Copyleft(c) 2022-present DVLab, GIEE, NTU, Taiwan ]
****************************************************************************/
#include "tensorMgr.h"
#include <iostream>
#include <algorithm>

using namespace std;

extern size_t verbose;

TensorMgr* tensorMgr = 0;

// @brief Reset the tensor manager
void TensorMgr::reset() {
    _tensorList.clear();
}

// @brief Add tensor to the tensor manager
// @param id
// @param str additional information (default: none)
QTensor<double>* TensorMgr::addTensor(const size_t& id, const string& str) {
    if (_tensorList.find(id) != _tensorList.end()) {
        return nullptr;
    }
    _tensorList[id] = {new QTensor<double>(), str};
    if (verbose >= 3) {
        cout << "Successfully added Tensor " << id << endl;
    }
    return _tensorList[id].tensor;
}

// @brief Remove tensor in the tensor manager
// @param id
void TensorMgr::removeTensor(const size_t& id) {
    if (_tensorList.find(id) == _tensorList.end()) {
        cerr << "[Error] The id provided does not exist!!" << endl;
    }
    if (_tensorList[id].tensor) {
        delete _tensorList[id].tensor;
    }
    _tensorList.erase(id);

    if (verbose >= 3) cout << "Successfully removed Tensor " << id << endl;

    return;
}

// @brief List infos of all tensors
void TensorMgr::printTensorMgr() const {
    for (const auto& [id, tsInfo] : _tensorList) {
        cout << "Tensor " << setw(4) << right << id << ": " << tsInfo << endl;
    }
    cout << "Total #Tensor: " << _tensorList.size() << endl;
}

// @brief Print tensor and its information
// @param id
// @param brief list information only (default = false)
void TensorMgr::printTensor(const size_t& id, bool brief = false) const {
    if (!brief) cout << *getTensor(id) << endl;
    cout << _tensorList.at(id) << endl;
}

// @brief Get the next available id in the tensor manager
size_t TensorMgr::nextID() {
    size_t id = 0;
    while (_tensorList.find(id) != _tensorList.end()) ++id;
    return id;
}

// @brief Check the equivalence of two tensors
// @param id1
// @param id2
// @param eps: two tensors are deemed equivalent if the cosine similarity is higher than 1 - eps (default to 1e-6)
bool TensorMgr::isEquivalent(const size_t& id1, const size_t& id2, const double& eps) const {
    if (getTensor(id1)->shape() != getTensor(id2)->shape()) return false;
    return cosineSimilarity(*getTensor(id1), *getTensor(id2)) >= (1 - eps);
}
// @brief Get the global norm of the two tensors. This function is only well-defined when two tensors are high in cosine similarity
// @param id1
// @param id2
double TensorMgr::getGlobalNorm(const size_t& id1, const size_t& id2) const {
    return globalNorm(*getTensor(id1), *getTensor(id2));
}

// @brief Get the global phase of the two tensors. This function is only well-defined when two tensors are high in cosine similarity
// @param id1
// @param id2
Phase TensorMgr::getGlobalPhase(const size_t& id1, const size_t& id2) const {
    return globalPhase(*getTensor(id1), *getTensor(id2));
}