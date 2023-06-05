/****************************************************************************
  FileName     [ simplify.h ]
  PackageName  [ simplifier ]
  Synopsis     [ Define class Simplifier structure ]
  Author       [ Design Verification Lab ]
  Copyright    [ Copyright(c) 2023 DVLab, GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef SIMPLIFY_H
#define SIMPLIFY_H

#include <memory>

#include "zxRules.h"  // for ZXRule
#include "zxoptimizer.h"

class ZXGraph;

class Simplifier {
public:
    Simplifier(ZXGraph* g) {
        _rule = nullptr;
        _simpGraph = g;
        _recipe.clear();
        hruleSimp();
    }
    Simplifier(std::unique_ptr<ZXRule> rule, ZXGraph* g) {
        _rule = std::move(rule);
        _simpGraph = g;
        _recipe.clear();
    }

    ZXRule* getRule() const { return _rule.get(); }

    void setRule(std::unique_ptr<ZXRule> rule) { _rule = std::move(rule); }

    void amend();
    // Simplification strategies
    int simp();
    int hadamardSimp();

    // Basic rules simplification
    int bialgSimp();
    int copySimp();
    int gadgetSimp();
    int hfusionSimp();
    int hruleSimp();
    int idSimp();
    int lcompSimp();
    int pivotSimp();
    int pivotBoundarySimp();
    int pivotGadgetSimp();
    int sfusionSimp();

    // action
    void toGraph();
    void toRGraph();
    int interiorCliffordSimp();
    int piCliffordSimp();
    int cliffordSimp();
    void fullReduce();
    void dynamicReduce();
    void symbolicReduce();

    // print function
    void printRecipe();
    void printOptimizer();

private:
    std::unique_ptr<ZXRule> _rule;
    ZXGraph* _simpGraph;
    std::vector<std::tuple<std::string, std::vector<int> > > _recipe;
};

#endif