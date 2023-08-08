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

#include "cmdParser.h"
#include "stop_token.hpp"
#include "zxRules.h"
#include "zxoptimizer.h"

extern size_t verbose;
extern ZXOPTimizer opt;
extern CmdParser cli;

class ZXGraph;

class Simplifier {
public:
    Simplifier(ZXGraph* g) : _rule{nullptr}, _simpGraph{g} {
        hruleSimp();
    }
    Simplifier(std::unique_ptr<ZXRule> rule, ZXGraph* g) : _rule{std::move(rule)}, _simpGraph{g} {}

    ZXRule* getRule() const { return _rule.get(); }

    void setRule(std::unique_ptr<ZXRule> rule) { _rule = std::move(rule); }

    void rewrite() { _rule->rewrite(_simpGraph); };
    void amend();

    template <typename Rule>
    size_t new_simp(const Rule& rule) {
        // Use hadamardSimp() instead
        assert(rule.name != "Hadamard Rule");

        if (verbose >= 5) std::cout << std::setw(30) << std::left << rule.name;
        if (verbose >= 8) std::cout << std::endl;

        std::vector<int> match_counts;

        size_t iterations = 0;
        for (int r2r = opt.getR2R(rule.name); !cli.stop_requested() && r2r > 0; r2r--) {
            std::vector<typename Rule::MatchType> matches = rule.findMatches(*_simpGraph);
            if (matches.empty()) {
                break;
            }
            match_counts.emplace_back(matches.size());
            iterations++;

            if (verbose >= 8) std::cout << "\nIteration " << iterations << ":" << std::endl
                                        << ">>>" << std::endl;
            rule.apply(*_simpGraph, matches);
            if (verbose >= 8) std::cout << "<<<" << std::endl;
        }

        _recipe.emplace_back(rule.name, match_counts);
        if (verbose >= 8) std::cout << "=> ";
        if (verbose >= 5) {
            std::cout << iterations << " iterations." << std::endl;
            for (size_t i = 0; i < match_counts.size(); i++) {
                std::cout << "  " << i + 1 << ") " << match_counts[i] << " matches" << std::endl;
            }
        }
        if (verbose >= 5) std::cout << "\n";

        return iterations;
    }

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
    void dynamicReduce(size_t tOptimal);
    void symbolicReduce();
    void partitionReduce(size_t numPartitions, size_t iterations);

    // print function
    void printRecipe();
    void printOptimizer();
    void getStepInfo(ZXGraph* g);

private:
    std::unique_ptr<ZXRule> _rule;
    ZXGraph* _simpGraph;
    std::vector<std::tuple<std::string, std::vector<int> > > _recipe;
};

#endif
