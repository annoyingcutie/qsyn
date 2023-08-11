/****************************************************************************
  FileName     [ trie.cpp ]
  PackageName  [ util ]
  Synopsis     [ User-defined trie data structure for parsing ]
  Author       [ Design Verification Lab ]
  Copyright    [ Copyright(c) 2023 DVLab, GIEE, NTU, Taiwan ]
****************************************************************************/

#include "./trie.hpp"

#include <cassert>

using namespace std;

namespace dvlab_utils {

bool Trie::insert(string const& word) {
    auto itr = _root.get();

    assert(itr != nullptr);

    for (auto& ch : word) {
        size_t idx = ch;
        if (itr->children[idx].get() == nullptr) {
            itr->children[idx] = make_unique<TrieNode>();
        }
        itr = itr->children[idx].get();
        itr->frequency++;
    }
    if (itr->isWord) return false;

    itr->isWord = true;
    return true;
}

optional<string> Trie::shortestUniquePrefix(string const& word) const {
    auto itr = _root.get();

    assert(itr != nullptr);

    size_t pos = 0;
    for (auto& ch : word) {
        pos++;
        itr = itr->children[(size_t)ch].get();
        if (itr == nullptr) return nullopt;
        if (itr->frequency == 1) break;
    }

    return word.substr(0, pos);
}

size_t Trie::frequency(string const& word) const {
    auto itr = _root.get();

    assert(itr != nullptr);

    for (auto& ch : word) {
        itr = itr->children[(size_t)ch].get();
        if (itr == nullptr) return 0;
    }

    return itr->frequency;
}

optional<string> Trie::findWithPrefix(string const& word) const {
    auto itr = _root.get();

    assert(itr != nullptr);
    string retStr = "";

    for (auto& ch : word) {
        itr = itr->children[(size_t)ch].get();
        if (itr == nullptr) return nullopt;
        retStr.push_back(ch);
    }

    if (itr->frequency > 1) return nullopt;

    while (!itr->isWord) {
        assert(!itr->children.empty());
        retStr.push_back(itr->children.begin()->first);
        itr = itr->children.begin()->second.get();
    }

    return retStr;
}

}  // namespace dvlab_utils