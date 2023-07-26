/****************************************************************************
  FileName     [ myConcepts.h ]
  PackageName  [ util ]
  Synopsis     [ User-defined concepts ]
  Author       [ Design Verification Lab ]
  Copyright    [ Copyright(c) 2023 DVLab, GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef MY_CONCEPTS_H
#define MY_CONCEPTS_H

#include <array>
#include <concepts>
#include <iostream>
#include <string>
#include <type_traits>

#include "rationalNumber.h"

template<class A>
struct is_fixed_array : std::false_type {};

// only works with arrays by specialization.
template<class T, std::size_t I>
struct is_fixed_array<std::array<T, I>> : std::true_type {};

template <typename T>
concept Arithmetic = std::is_arithmetic_v<T>;

template <typename T>
concept Unitless = requires(T t) {
    Arithmetic<T> == true || std::same_as<T, Rational> == true;
};

template <class T>
concept StringLike = std::is_convertible_v<T, std::string_view>;

template <typename T>
concept Printable = requires(T t) {
    { std::cout << t } -> std::same_as<std::ostream&>;
};

template <typename T>
concept PrintableButNotStringLike = Printable<T> && !StringLike<T>;

#endif  // MY_CONCEPTS_H