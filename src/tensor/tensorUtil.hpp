/****************************************************************************
  FileName     [ tensorUtil.hpp ]
  PackageName  [ tensor ]
  Synopsis     [ Define Tensor base class interface ]
  Author       [ Design Verification Lab ]
  Copyright    [ Copyright(c) 2023 DVLab, GIEE, NTU, Taiwan ]
****************************************************************************/
#pragma once

#include <cstddef>
#include <vector>
#include <xtensor/xstorage.hpp>

using TensorShape = xt::svector<size_t>;
using TensorIndex = std::vector<size_t>;
using TensorAxisList = std::vector<size_t>;

TensorAxisList concatAxisList(const TensorAxisList& ax1, const TensorAxisList& ax2);
void printAxisList(const TensorAxisList& ax);

bool isDisjoint(const TensorAxisList& ax1, const TensorAxisList& ax2);
