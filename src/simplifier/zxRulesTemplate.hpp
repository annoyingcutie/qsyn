/************************************************************
  FileName     [ zxRulesTemplate.hpp ]
  PackageName  [ simplifier ]
  Synopsis     [ Define class Simplifier structure ]
  Author       [ Design Verification Lab ]
  Copyright    [ Copyright(c) 2023 DVLab, GIEE, NTU, Taiwan ]
****************************************************************************/

#pragma once

#include <vector>

#include "zxDef.h"
#include "zxGraph.h"

struct ZXOperation {
    std::vector<ZXVertex*> verticesToAdd;
    std::vector<EdgePair> edgesToAdd;
    std::vector<EdgePair> edgesToRemove;
    std::vector<ZXVertex*> verticesToRemove;
};

template <typename _MatchType>
class ZXRuleTemplate {
public:
    using MatchType = _MatchType;

    ZXRuleTemplate(const std::string& _name) : name(_name) {}

    virtual std::vector<MatchType> findMatches(const ZXGraph& graph) const = 0;
    virtual void apply(ZXGraph& graph, const std::vector<MatchType>& matches) const = 0;

    const std::string name;

protected:
    void update(ZXGraph& graph, const ZXOperation& op) const {
        // TODO: add vertices first
        for (auto& edge : op.edgesToAdd) {
            auto [v0, v1] = std::get<0>(edge);
            EdgeType edgeType = std::get<1>(edge);
            graph.addEdge(v0, v1, edgeType);
        }
        graph.removeEdges(op.edgesToRemove);
        graph.removeVertices(op.verticesToRemove);

        graph.removeIsolatedVertices();
    }
};

class IdRemovalRule : public ZXRuleTemplate<std::tuple<ZXVertex*, ZXVertex*, ZXVertex*, EdgeType>> {
public:
    using MatchType = ZXRuleTemplate::MatchType;

    IdRemovalRule() : ZXRuleTemplate("Identity Removal Rule") {}

    std::vector<MatchType> findMatches(const ZXGraph& graph) const override;
    void apply(ZXGraph& graph, const std::vector<MatchType>& matches) const override;
};

class BialgebraRule : public ZXRuleTemplate<EdgePair> {
public:
    using MatchType = ZXRuleTemplate::MatchType;

    BialgebraRule() : ZXRuleTemplate("Bialgebra Rule") {}

    std::vector<MatchType> findMatches(const ZXGraph& graph) const override;
    void apply(ZXGraph& graph, const std::vector<MatchType>& matches) const override;

private:
    bool has_dupicate(std::vector<ZXVertex*> vec) const;
};

class LocalComplementRule : public ZXRuleTemplate<std::pair<ZXVertex*, std::vector<ZXVertex*>>> {
public:
    using MatchType = ZXRuleTemplate::MatchType;

    LocalComplementRule() : ZXRuleTemplate("Local Complementation Rule") {}

    std::vector<MatchType> findMatches(const ZXGraph& graph) const override;
    void apply(ZXGraph& graph, const std::vector<MatchType>& matches) const override;
};
