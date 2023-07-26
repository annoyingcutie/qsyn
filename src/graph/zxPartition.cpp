/****************************************************************************
  FileName     [ zxPartition.cpp ]
  PackageName  [ graph ]
  Synopsis     [ Implements the split function ]
  Author       [ Design Verification Lab ]
  Copyright    [ Copyright(c) 2023 DVLab, GIEE, NTU, Taiwan ]
****************************************************************************/

#include "zxPartition.h"

#include <algorithm>
#include <cstdint>
#include <iomanip>
#include <stack>
#include <unordered_map>
#include <utility>

#include "zxDef.h"
#include "zxGraph.h"
#include "zxGraphMgr.h"

extern ZXGraphMgr zxGraphMgr;

/*****************************************************/
/*   class ZXGraph partition functions.              */
/*****************************************************/

const int CUT_BOUNDARY_QUBIT_ID = INT_MIN;

struct DirectionalZXCutHash {
    size_t operator()(const ZXCut& cut) const {
        auto [v1, v2, edgeType] = cut;
        return std::hash<ZXVertex*>()(v1) ^ std::hash<ZXVertex*>()(v2) ^ std::hash<EdgeType>()(edgeType);
    };
};

std::pair<std::vector<ZXGraph*>, std::vector<ZXCut>>
ZXGraph::createSubgraphs(ZXPartitionStrategy partitionStrategy, size_t numPartitions) {
    std::vector<ZXVertexList> partitions = partitionStrategy(*this, numPartitions);
    std::vector<ZXGraph*> subgraphs;
    // stores the two sides of the cut and the edge type
    ZXCutSet innerCuts;
    // stores the two boundary vertices and the edge type corresponding to the cut
    std::vector<ZXCut> outerCuts;
    std::unordered_map<ZXCut, ZXVertex*, DirectionalZXCutHash> cutToBoundary;

    ZXVertexList primaryInputs = getInputs();
    ZXVertexList primaryOutputs = getOutputs();

    for (auto& partition : partitions) {
        ZXVertexList subgraphInputs;
        ZXVertexList subgraphOutputs;
        std::vector<ZXVertex*> boundaryVertices;

        size_t nextVertexId = 0;
        for (const auto& vertex : partition) {
            vertex->setId(nextVertexId);
            nextVertexId++;
        }

        for (const auto& vertex : partition) {
            if (primaryInputs.contains(vertex)) subgraphInputs.insert(vertex);
            if (primaryOutputs.contains(vertex)) subgraphOutputs.insert(vertex);

            std::cerr << "vertex: " << vertex->getId() << std::endl;
            std::vector<NeighborPair> neighborsToRemove;
            std::vector<NeighborPair> neighborsToAdd;
            for (const auto& [neighbor, edgeType] : vertex->getNeighbors()) {
                std::cerr << "\tneighbor: " << neighbor->getId() << std::endl;
                if (!partition.contains(neighbor)) {
                    ZXVertex* boundary = new ZXVertex(nextVertexId++, CUT_BOUNDARY_QUBIT_ID, VertexType::BOUNDARY);
                    innerCuts.insert({vertex, neighbor, edgeType});
                    cutToBoundary[{vertex, neighbor, edgeType}] = boundary;

                    neighborsToRemove.push_back({neighbor, edgeType});
                    neighborsToAdd.push_back({boundary, edgeType});

                    boundary->addNeighbor(vertex, edgeType);

                    boundaryVertices.push_back(boundary);
                    subgraphOutputs.insert(boundary);
                }
            }

            for (const auto& neighborPair : neighborsToAdd) {
                vertex->addNeighbor(neighborPair);
            }
            for (const auto& neighborPair : neighborsToRemove) {
                vertex->removeNeighbor(neighborPair);
            }
        }

        for (const auto& vertex : boundaryVertices) {
            partition.insert(vertex);
        }
        subgraphs.push_back(new ZXGraph(partition, subgraphInputs, subgraphOutputs, zxGraphMgr.getNextID()));
    }

    std::cerr << "subgraphs: " << std::endl;
    for (auto g : subgraphs) {
        g->printVertices();
    }

    for (auto [cut, b] : cutToBoundary) {
        auto [v1, v2, e] = cut;
        std::cerr << v1 << " " << v2 << "->" << b << std::endl;
    }

    for (auto [v1, v2, edgeType] : innerCuts) {
        ZXVertex* boundary1 = cutToBoundary[{v1, v2, edgeType}];
        ZXVertex* boundary2 = cutToBoundary[{v2, v1, edgeType}];

        std::cerr << "====================" << std::endl;
        std::cout << boundary1 << " " << boundary2 << std::endl;
        std::cerr << "v1: " << std::endl;
        for (auto [n, e] : v1->getNeighbors()) {
            std::cerr << n << " ";
        }
        std::cerr << std::endl;
        std::cerr << "v2: " << std::endl;
        for (auto [n, e] : v2->getNeighbors()) {
            std::cerr << n << " ";
        }
        std::cerr << std::endl;

        outerCuts.push_back({boundary1, boundary2, edgeType});
    }

    return {subgraphs, outerCuts};
}

/**
 * @brief Creates a new ZXGraph from a list of subgraphs and a list of cuts
 *        between the subgraphs. Deletes the subgraphs after merging.
 *
 * @param subgraphs The list of subgraphs to merge
 * @param cuts The list of cuts between the subgraph (boundary vertices)
 *
 */
ZXGraph* ZXGraph::fromSubgraphs(const std::vector<ZXGraph*>& subgraphs, const std::vector<ZXCut>& cuts) {
    // TODO: Implement from subgraph
    ZXVertexList vertices;
    ZXVertexList inputs;
    ZXVertexList outputs;

    for (auto subgraph : subgraphs) {
        vertices.insert(subgraph->getVertices().begin(), subgraph->getVertices().end());
        inputs.insert(subgraph->getInputs().begin(), subgraph->getInputs().end());
        outputs.insert(subgraph->getOutputs().begin(), subgraph->getOutputs().end());
    }

    for (auto [b1, b2, edgeType] : cuts) {
        ZXVertex* v1 = b1->getFirstNeighbor().first;
        ZXVertex* v2 = b2->getFirstNeighbor().first;
        vertices.erase(b1);
        vertices.erase(b2);
        v1->removeNeighbor(b1, edgeType);
        v2->removeNeighbor(b2, edgeType);
        v1->addNeighbor(v2, edgeType);
        v2->addNeighbor(v1, edgeType);
    }

    for (auto subgraph : subgraphs) {
        // NOTE: ownership of the vertices is transferred to the merged graph
        subgraph->reset();
        delete subgraph;
    }

    return new ZXGraph(vertices, inputs, outputs, zxGraphMgr.getNextID());
}

/*****************************************************/
/*  ZXGraph partition strategies.                    */
/*****************************************************/

std::pair<ZXVertexList, ZXVertexList> _klBiPartition(ZXVertexList vertices);

/* @brief Recursively partition the graph into numPartitions partitions using the Kernighan-Lin algorithm.
 * @param graph The graph to partition.
 * @param numPartitions The number of partitions to split the graph into.
 * @return A vector of vertex lists, each representing a partition.
 */
std::vector<ZXVertexList> klPartition(const ZXGraph& graph, size_t numPartitions) {
    std::vector<ZXVertexList> partitions = {graph.getVertices()};
    size_t count = 1;
    while (count < numPartitions) {
        std::vector<ZXVertexList> newPartitions;
        for (auto& partition : partitions) {
            auto [p1, p2] = _klBiPartition(partition);
            partition = p1;
            newPartitions.push_back(p2);
            if (++count == numPartitions) break;
        }
        partitions.insert(partitions.end(), newPartitions.begin(), newPartitions.end());
    }
    return partitions;
}

std::pair<ZXVertexList, ZXVertexList> _klBiPartition(ZXVertexList vertices) {
    using SwapPair = std::pair<ZXVertex*, ZXVertex*>;

    ZXVertexList partition1 = ZXVertexList();
    ZXVertexList partition2 = ZXVertexList();

    bool toggle = false;
    for (auto v : vertices) {
        if (toggle) {
            partition1.insert(v);
        } else {
            partition2.insert(v);
        }
        toggle = !toggle;
    }

    std::unordered_map<ZXVertex*, int> dValues;
    int cumulativeGain;
    std::stack<SwapPair> swapHistory;
    int bestCumulativeGain;
    size_t bestIteration;
    std::unordered_set<ZXVertex*> lockedVertices;

    auto computeD = [&]() {
        for (auto& v : vertices) {
            int internal_cost = 0;
            int external_cost = 0;

            ZXVertexList& myPartition = partition1.contains(v) ? partition1 : partition2;
            ZXVertexList& otherPartition = partition1.contains(v) ? partition2 : partition1;

            for (auto& [neighbor, edge] : v->getNeighbors()) {
                if (myPartition.contains(neighbor)) {
                    internal_cost++;
                } else if (otherPartition.contains(neighbor)) {
                    external_cost++;
                }
            }

            dValues[v] = external_cost - internal_cost;
        }
    };

    auto swapOnce = [&]() {
        SwapPair bestSwap = {nullptr, nullptr};
        int bestSwapGain = INT_MIN;

        for (auto& v1 : partition1) {
            if (lockedVertices.contains(v1)) continue;
            for (auto& v2 : partition2) {
                if (lockedVertices.contains(v2)) continue;
                int swapGain = dValues[v1] + dValues[v2] - 2 * (v1->isNeighbor(v2) ? 1 : 0);
                if (swapGain > bestSwapGain) {
                    bestSwapGain = swapGain;
                    bestSwap = {v1, v2};
                }
            }
        }

        auto [swap1, swap2] = bestSwap;
        partition1.erase(swap1);
        partition2.erase(swap2);
        partition1.insert(swap2);
        partition2.insert(swap1);
        lockedVertices.insert(swap1);
        lockedVertices.insert(swap2);

        for (auto& v1 : partition1) {
            if (lockedVertices.contains(v1)) continue;
            dValues[v1] += 2 * (v1->isNeighbor(swap1) ? 1 : 0) - 2 * (v1->isNeighbor(swap2) ? 1 : 0);
        }
        for (auto& v2 : partition2) {
            if (lockedVertices.contains(v2)) continue;
            dValues[v2] += 2 * (v2->isNeighbor(swap2) ? 1 : 0) - 2 * (v2->isNeighbor(swap1) ? 1 : 0);
        }

        cumulativeGain += bestSwapGain;
        swapHistory.push(bestSwap);
        if (cumulativeGain >= bestCumulativeGain) {
            bestCumulativeGain = cumulativeGain;
            bestIteration = swapHistory.size();
        }
    };

    size_t iteration = 0;
    while (true) {
        cumulativeGain = 0;
        swapHistory = std::stack<SwapPair>();
        bestCumulativeGain = INT_MIN;
        bestIteration = 0;
        lockedVertices.clear();
        computeD();

        // OPTIMIZE: decide a better stopping condition
        for (size_t _ = 0; _ < partition1.size() - 1; _++) {
            swapOnce();
        }
        // for (size_t _ = 0; _ < partition1.size() / 2; _++) {
        //     swapOnce();
        // }

        // OPTIMIZE: decide a better stopping condition
        if (bestCumulativeGain <= 0) {
            break;
        }
        // if (bestCumulativeGain <= partition1.size() / 10) {
        //     break;
        // }

        // NOTE: undo until best iteration
        while (swapHistory.size() > bestIteration) {
            auto [swap1, swap2] = swapHistory.top();
            swapHistory.pop();
            partition1.erase(swap2);
            partition2.erase(swap1);
            partition1.insert(swap1);
            partition2.insert(swap2);
        }
    }

    return std::make_pair(partition1, partition2);
}
