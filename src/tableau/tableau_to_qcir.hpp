/****************************************************************************
  PackageName  [ tableau ]
  Synopsis     [ Define pauli rotation class ]
  Author       [ Design Verification Lab ]
  Copyright    [ Copyright(c) 2023 DVLab, GIEE, NTU, Taiwan ]
****************************************************************************/

#pragma once

#include "qcir/qcir.hpp"
#include "tableau/pauli_rotation.hpp"
#include "tableau/qcir_to_tableau.hpp"
#include "tableau/stabilizer_tableau.hpp"

namespace qsyn {

namespace experimental {

qcir::QCir to_qcir(StabilizerTableau clifford, StabilizerTableauExtractor const& extractor = AGExtractor{});
qcir::QCir to_qcir(PauliRotation const& pauli_rotation);
qcir::QCir to_qcir(Tableau const& tableau, StabilizerTableauExtractor const& extractor = AGExtractor{});

}  // namespace experimental

}  // namespace qsyn
