/****************************************************************************
  FileName     [ pivotInterface.cpp ]
  PackageName  [ simplifier ]
  Synopsis     [ Common Interface to Pivot-like Rule ]
  Author       [ Mu-Te (Joshua) Lau ]
  Copyright    [ Copyleft(c) 2022-present DVLab, GIEE, NTU, Taiwan ]
****************************************************************************/

#include <iostream>
#include <numbers>
#include <vector>

#include "zxRules.h"
using namespace std;

extern size_t verbose;

/**
 * @brief Generate Rewrite format from `_matchTypeVec`
 *
 * @param g
 */
void PivotInterface::rewrite(ZXGraph* g) {
    reset();
    preprocess(g);
    
    
    for (auto& m : _matchTypeVec) {
        if (verbose >= 5) {
            cout << "> rewrite...\n";
            cout << "vs: " << m[0]->getId() << "\tvt: " << m[1]->getId() << endl;
        }
        
        vector<ZXVertex*> n0, n1, n2;
        vector<ZXVertex*> tmp0 = m[0]->getCopiedNeighbors();
        vector<ZXVertex*> tmp1 = m[1]->getCopiedNeighbors();

        tmp0.erase(std::find(tmp0.begin(), tmp0.end(), m[1]));
        tmp1.erase(std::find(tmp1.begin(), tmp1.end(), m[0]));

        set_intersection(tmp0.begin(), tmp0.end(), tmp1.begin(), tmp1.end(), back_inserter(n2));

        set_difference(tmp0.begin(), tmp0.end(), n2.begin(), n2.end(), back_inserter(n0));
        set_difference(tmp1.begin(), tmp1.end(), n2.begin(), n2.end(), back_inserter(n1));

        tmp0.clear();
        tmp1.clear();

        // Add edge table
        for (const auto& s : n0) {
            for (const auto& t : n1) {
                _edgeTableKeys.push_back(make_pair(s, t));
                _edgeTableValues.push_back(make_pair(0, 1));
            }
            for (const auto& t : n2) {
                _edgeTableKeys.push_back(make_pair(s, t));
                _edgeTableValues.push_back(make_pair(0, 1));
            }
        }
        for (const auto& s : n1) {
            for (const auto& t : n2) {
                _edgeTableKeys.push_back(make_pair(s, t));
                _edgeTableValues.push_back(make_pair(0, 1));
            }
        }

        // REVIEW - check if not ground
        for (const auto& v : n0) v->setPhase(v->getPhase() + m[1]->getPhase()); 
        for (const auto& v : n1) v->setPhase(v->getPhase() + m[0]->getPhase());
        for (const auto& v : n2) v->setPhase(v->getPhase() + m[0]->getPhase() + m[1]->getPhase() + Phase(1));

        for (int i = 0; i < 2; ++i) _removeVertices.push_back(m[i]);
    }
}