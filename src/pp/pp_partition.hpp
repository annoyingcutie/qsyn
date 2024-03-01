/****************************************************************************
  PackageName  [ pp ]
  Synopsis     [ Implement partition algorithm ]
  Author       [ Design Verification Lab ]
  Copyright    [ Copyright(c) 2023 DVLab, GIEE, NTU, Taiwan ]
****************************************************************************/

#pragma once

#include "util/boolean_matrix.hpp"

namespace qsyn::pp {
// Todo: move them to pp.hpp
using Partition  = dvlab::BooleanMatrix;
using term       = dvlab::BooleanMatrix::Row;
using Partitions = std::vector<dvlab::BooleanMatrix>;
using Wires      = dvlab::BooleanMatrix;
using HMAP       = std::vector<std::pair<dvlab::BooleanMatrix, size_t>>;

class Partitioning {
public:
    Partitioning(dvlab::BooleanMatrix poly, size_t n, size_t a) : _variable(n), _qubit_num{n + a}, _poly(poly){};

    void initial(dvlab::BooleanMatrix poly, size_t n, size_t a);
    bool independant_oracle(Partition, term);

    // Greedy partitiion
    Partitions greedy_partitioning(HMAP h_map, size_t rank);
    Partitions greedy_partitioning_routine(Partitions partitions, Wires wires, size_t rank);

    // Matroid partition

    // Get function

    // Print function
    void print_poly() const { _poly.print_matrix(); };

private:
    size_t _variable;
    size_t _qubit_num;
    Partitions _partitions;
    dvlab::BooleanMatrix _poly;
};

}  // namespace qsyn::pp
