/****************************************************************************
  FileName     [ tensorMgr.hpp ]
  PackageName  [ tensor ]
  Synopsis     [ Define class TensorMgr structure ]
  Author       [ Design Verification Lab ]
  Copyright    [ Copyright(c) 2023 DVLab, GIEE, NTU, Taiwan ]
****************************************************************************/
#pragma once

#include <iosfwd>
#include <map>
#include <string>

#include "./qtensor.hpp"
#include "util/dataStructureManager.hpp"
#include "util/phase.hpp"

template <typename T>
class QTensor;

using TensorMgr = dvlab_utils::DataStructureManager<QTensor<double>>;
