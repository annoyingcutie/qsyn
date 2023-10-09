/****************************************************************************
  PackageName  [ simplifier ]
  Synopsis     [ State Copy Rule Definition ]
  Author       [ Design Verification Lab ]
  Copyright    [ Copyright(c) 2023 DVLab, GIEE, NTU, Taiwan ]
****************************************************************************/

#include "./zx_rules_template.hpp"
#include "tl/enumerate.hpp"

using namespace qsyn::zx;

using MatchType = StateCopyRule::MatchType;

/**
 * @brief Find spiders with a 0 or pi phase that have a single neighbor, and copies them through. Assumes that all the spiders are green and maximally fused.
 *
 * @param graph The graph to be matched.
 */
std::vector<MatchType> StateCopyRule::find_matches(ZXGraph const& graph) const {
    std::vector<MatchType> matches;

    std::unordered_map<ZXVertex*, size_t> vertex2idx;

    std::unordered_map<size_t, size_t> id2idx;
    for (auto const& [count, v] : tl::views::enumerate(graph.get_vertices())) {
        vertex2idx[v] = count;
    }

    std::vector<bool> valid_vertex(graph.get_num_vertices(), true);

    for (auto const& v : graph.get_vertices()) {
        if (!valid_vertex[vertex2idx[v]]) continue;

        if (v->get_type() != VertexType::z) {
            valid_vertex[vertex2idx[v]] = false;
            continue;
        }
        if (v->get_phase() != Phase(0) && v->get_phase() != Phase(1)) {
            valid_vertex[vertex2idx[v]] = false;
            continue;
        }
        if (graph.get_num_neighbors(v) != 1) {
            valid_vertex[vertex2idx[v]] = false;
            continue;
        }

        ZXVertex* pi_neighbor = graph.get_first_neighbor(v).first;
        if (pi_neighbor->get_type() != VertexType::z) {
            valid_vertex[vertex2idx[v]] = false;
            continue;
        }
        std::vector<ZXVertex*> apply_neighbors;
        for (auto const& [nebOfPiNeighbor, _] : graph.get_neighbors(pi_neighbor)) {
            if (nebOfPiNeighbor != v)
                apply_neighbors.emplace_back(nebOfPiNeighbor);
            valid_vertex[vertex2idx[nebOfPiNeighbor]] = false;
        }
        matches.emplace_back(make_tuple(v, pi_neighbor, apply_neighbors));
    }

    return matches;
}

void StateCopyRule::apply(ZXGraph& graph, std::vector<MatchType> const& matches) const {
    ZXOperation op;

    // Need to update global scalar and phase
    for (auto& match : matches) {
        ZXVertex* npi                    = get<0>(match);
        ZXVertex* a                      = get<1>(match);
        std::vector<ZXVertex*> neighbors = get<2>(match);
        op.vertices_to_remove.emplace_back(npi);
        op.vertices_to_remove.emplace_back(a);
        for (auto neighbor : neighbors) {
            if (neighbor->get_type() == VertexType::boundary) {
                ZXVertex* new_v  = graph.add_vertex(neighbor->get_qubit(), VertexType::z, npi->get_phase());
                bool simple_edge = graph.get_first_neighbor(neighbor).second == EdgeType::simple;

                op.edges_to_remove.emplace_back(std::make_pair(a, neighbor), graph.get_first_neighbor(neighbor).second);

                // new to Boundary
                op.edges_to_add.emplace_back(std::make_pair(new_v, neighbor), simple_edge ? EdgeType::hadamard : EdgeType::simple);

                // a to new
                op.edges_to_add.emplace_back(std::make_pair(a, new_v), EdgeType::hadamard);

                new_v->set_col((neighbor->get_col() + a->get_col()) / 2);

            } else {
                neighbor->set_phase(npi->get_phase() + neighbor->get_phase());
            }
        }
    }

    _update(graph, op);
}
