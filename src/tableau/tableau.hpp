/****************************************************************************
  PackageName  [ tableau ]
  Synopsis     [ Define tableau base class and its derived classes ]
  Author       [ Design Verification Lab ]
  Copyright    [ Copyright(c) 2023 DVLab, GIEE, NTU, Taiwan ]
****************************************************************************/

#pragma once

#include <fmt/core.h>
#include <fmt/format.h>

#include <cstddef>
#include <iostream>
#include <ranges>
#include <string>
#include <sul/dynamic_bitset.hpp>

#include "tableau/pauli_rotation.hpp"

namespace qsyn {

namespace experimental {

class StabilizerTableau : public PauliProductTrait<StabilizerTableau> {
public:
    StabilizerTableau(size_t n_qubits) : _rotations(2 * n_qubits, PauliProduct(std::string(n_qubits, 'I'))) {
        for (size_t i = 0; i < n_qubits; ++i) {
            _rotations[stabilizer_idx(i)].set(i, PauliType::Z);
            _rotations[destabilizer_idx(i)].set(i, PauliType::X);
        }
    }

    inline size_t n_qubits() const {
        return _rotations.size() / 2;
    }

    inline size_t stabilizer_idx(size_t qubit) const {
        return qubit;
    }
    inline size_t destabilizer_idx(size_t qubit) const {
        return qubit + n_qubits();
    }

    StabilizerTableau& h(size_t qubit) override;
    StabilizerTableau& s(size_t qubit) override;
    StabilizerTableau& cx(size_t ctrl, size_t targ) override;

    StabilizerTableau& measure_z(size_t qubit);

    std::string to_string() const;
    std::string to_bit_string() const;

    inline PauliProduct const& stabilizer(size_t qubit) const {
        return _rotations[stabilizer_idx(qubit)];
    }
    inline PauliProduct const& destabilizer(size_t qubit) const {
        return _rotations[destabilizer_idx(qubit)];
    }
    inline PauliProduct& stabilizer(size_t qubit) {
        return _rotations[stabilizer_idx(qubit)];
    }
    inline PauliProduct& destabilizer(size_t qubit) {
        return _rotations[destabilizer_idx(qubit)];
    }

    bool operator==(StabilizerTableau const& rhs) const {
        return _rotations == rhs._rotations;
    }
    bool operator!=(StabilizerTableau const& rhs) const {
        return !(*this == rhs);
    }

private:
    std::vector<PauliProduct> _rotations;
};

}  // namespace experimental

}  // namespace qsyn

template <>
struct fmt::formatter<qsyn::experimental::StabilizerTableau> {
    char presentation = 'c';
    constexpr auto parse(format_parse_context& ctx) {
        auto it = ctx.begin(), end = ctx.end();
        if (it != end && (*it == 'c' || *it == 'b')) presentation = *it++;
        if (it != end && *it != '}') throw format_error("invalid format");
        return it;
    }

    template <typename FormatContext>
    auto format(const qsyn::experimental::StabilizerTableau& tableau, FormatContext& ctx) {
        return presentation == 'c' ? format_to(ctx.out(), "{}", tableau.to_string())
                                   : format_to(ctx.out(), "{}", tableau.to_bit_string());
    }
};
