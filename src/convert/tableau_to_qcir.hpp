/****************************************************************************
  PackageName  [ tableau ]
  Synopsis     [ Define pauli rotation class ]
  Author       [ Design Verification Lab ]
  Copyright    [ Copyright(c) 2023 DVLab, GIEE, NTU, Taiwan ]
****************************************************************************/

#pragma once

#include "qcir/qcir.hpp"
#include "tableau/pauli_rotation.hpp"
#include "tableau/tableau.hpp"

namespace qsyn {

namespace experimental {

struct PauliRotationsSynthesisStrategy {
public:
    virtual ~PauliRotationsSynthesisStrategy() = default;

    virtual qcir::QCir synthesize(std::vector<PauliRotation> const& rotations) const = 0;
};

struct NaivePauliRotationsSynthesisStrategy : public PauliRotationsSynthesisStrategy {
    qcir::QCir synthesize(std::vector<PauliRotation> const& rotations) const override;
};

qcir::QCir to_qcir(StabilizerTableau clifford, StabilizerTableauSynthesisStrategy const& extractor = HOptSynthesisStrategy{});
qcir::QCir to_qcir(std::vector<PauliRotation> const& pauli_rotations);
qcir::QCir to_qcir(Tableau const& tableau, StabilizerTableauSynthesisStrategy const& extractor = HOptSynthesisStrategy{});

}  // namespace experimental

}  // namespace qsyn
