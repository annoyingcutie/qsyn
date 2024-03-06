/****************************************************************************
  PackageName  [ tensor ]
  Synopsis     [ Define class Tensor structure ]
  Author       [ Design Verification Lab ]
  Copyright    [ Copyright(c) 2023 DVLab, GIEE, NTU, Taiwan ]
****************************************************************************/

#pragma once

#include <iostream>
#include <fmt/core.h>
#include <fmt/ostream.h>
#include <math.h>
#include <spdlog/spdlog.h>
#include <boost/dynamic_bitset.hpp>
#include <string>
#include <array>

#include <cassert>
#include <complex>
#include <cstddef>
#include <iosfwd>
#include <tuple>
#include <unordered_map>
#include <vector>
#include <xtensor-blas/xlinalg.hpp>
#include <xtensor/xadapt.hpp>
#include <xtensor/xarray.hpp>
#include <xtensor/xcsv.hpp>
#include <xtensor/xio.hpp>
#include <xtensor/xnpy.hpp>

#include "./tensor_util.hpp"
#include "util/util.hpp"

constexpr double PI = 3.1415926919;

namespace qsyn::tensor {

template <typename DT>
class Tensor {
protected:
    using DataType     = DT;
    using InternalType = xt::xarray<DataType>;

public:
    // NOTE - the initialization of _tensor must use (...) because {...} is list initialization
    Tensor(xt::nested_initializer_list_t<DT, 0> il) : _tensor(il) { reset_axis_history(); }
    Tensor(xt::nested_initializer_list_t<DT, 1> il) : _tensor(il) { reset_axis_history(); }
    Tensor(xt::nested_initializer_list_t<DT, 2> il) : _tensor(il) { reset_axis_history(); }
    Tensor(xt::nested_initializer_list_t<DT, 3> il) : _tensor(il) { reset_axis_history(); }
    Tensor(xt::nested_initializer_list_t<DT, 4> il) : _tensor(il) { reset_axis_history(); }
    Tensor(xt::nested_initializer_list_t<DT, 5> il) : _tensor(il) { reset_axis_history(); }

    virtual ~Tensor() = default;

    Tensor(TensorShape const& shape) : _tensor(shape) { reset_axis_history(); }
    Tensor(TensorShape&& shape) : _tensor(std::move(shape)) { reset_axis_history(); }

    template <typename From>
    requires std::convertible_to<From, InternalType>
    Tensor(From const& internal) : _tensor(internal) { reset_axis_history(); }

    template <typename From>
    requires std::convertible_to<From, InternalType>
    Tensor(From&& internal) : _tensor(std::forward<InternalType>(internal)) { reset_axis_history(); }

    template <typename... Args>
    DT& operator()(Args const&... args);
    template <typename... Args>
    const DT& operator()(Args const&... args) const;

    DT& operator[](TensorIndex const& i);
    const DT& operator[](TensorIndex const& i) const;

    virtual bool operator==(Tensor<DT> const& rhs) const;
    virtual bool operator!=(Tensor<DT> const& rhs) const;

    template <typename U>
    friend std::ostream& operator<<(std::ostream& os, Tensor<U> const& t);  // different typename to avoid shadowing

    virtual Tensor<DT>& operator+=(Tensor<DT> const& rhs);
    virtual Tensor<DT>& operator-=(Tensor<DT> const& rhs);
    virtual Tensor<DT>& operator*=(Tensor<DT> const& rhs);
    virtual Tensor<DT>& operator/=(Tensor<DT> const& rhs);

    virtual Tensor<DT>& operator*(Tensor<DT> const& rhs);

    template <typename U>
    friend Tensor<U> operator+(Tensor<U> lhs, Tensor<U> const& rhs);
    template <typename U>
    friend Tensor<U> operator-(Tensor<U> lhs, Tensor<U> const& rhs);
    template <typename U>
    friend Tensor<U> operator*(Tensor<U> lhs, Tensor<U> const& rhs);
    template <typename U>
    friend Tensor<U> operator/(Tensor<U> lhs, Tensor<U> const& rhs);

    size_t dimension() const { return _tensor.dimension(); }
    std::vector<size_t> shape() const;

    void reset_axis_history();
    size_t get_new_axis_id(size_t const& old_id);

    template <typename U>
    friend double inner_product(Tensor<U> const& t1, Tensor<U> const& t2);

    template <typename U>
    friend double cosine_similarity(Tensor<U> const& t1, Tensor<U> const& t2);

    //template <typename DT>
    double trace_distance(Tensor<DT>& t1, Tensor<DT>& t2);

    template <typename U>
    friend Tensor<U> tensordot(Tensor<U> const& t1, Tensor<U> const& t2,
                               TensorAxisList const& ax1, TensorAxisList const& ax2);

    template <typename U>
    friend Tensor<U> tensor_product_pow(Tensor<U> const& t, size_t n);

    template <typename U>
    friend bool is_partition(Tensor<U> const& t, TensorAxisList const& axes1, TensorAxisList const& axes2);

    Tensor<DT> to_matrix(TensorAxisList const& row_axes, TensorAxisList const& col_axis);

    template <typename U>
    friend Tensor<U> direct_sum(Tensor<U> const& t1, Tensor<U> const& t2);

    void reshape(TensorShape const& shape);
    Tensor<DT> transpose(TensorAxisList const& perm) const;
    void adjoint();
    void sqrt();

    std::vector<std::string> binary_prod(int n);
    std::vector<Tensor<DT>> create_unitaries(const std::vector<Tensor<DT>>  base, int limit);
    void save_unitaries(const std::vector<Tensor<DT>>& unitaries, const std::string& filename);

    Tensor<DT> random_u(double x,double y, double z);
    std::vector<std::complex<double>> u_to_bloch(Tensor<DT> u);

    std::complex<double> determinant();
    std::complex<double> trace();
    auto eigen();  // TODO - check the return type, may be std::pair<value, vector>


    Tensor<DT> to_su2(Tensor<DT> &u);
   

    void SK_decompose();

    bool tensor_read(std::string const&);
    bool tensor_write(std::string const&);

protected:
    friend struct fmt::formatter<Tensor>;
    InternalType _tensor;
    std::unordered_map<size_t, size_t> _axis_history;
};

//------------------------------
// Operators
//------------------------------

template <typename DT>
template <typename... Args>
DT& Tensor<DT>::operator()(Args const&... args) {
    return _tensor(args...);
}
template <typename DT>
template <typename... Args>
const DT& Tensor<DT>::operator()(Args const&... args) const {
    return _tensor(args...);
}

template <typename DT>
DT& Tensor<DT>::operator[](TensorIndex const& i) {
    return _tensor[i];
}
template <typename DT>
const DT& Tensor<DT>::operator[](TensorIndex const& i) const {
    return _tensor[i];
}

template <typename DT>
bool Tensor<DT>::operator==(Tensor<DT> const& rhs) const {
    return _tensor == rhs._tensor;
}
template <typename DT>
bool Tensor<DT>::operator!=(Tensor<DT> const& rhs) const {
    return _tensor != rhs._tensor;
}

template <typename U>
std::ostream& operator<<(std::ostream& os, Tensor<U> const& t) {
    return os << t._tensor;
}

template <typename DT>
Tensor<DT>& Tensor<DT>::operator+=(Tensor<DT> const& rhs) {
    _tensor += rhs._tensor;
    return *this;
}

template <typename DT>
Tensor<DT>& Tensor<DT>::operator-=(Tensor<DT> const& rhs) {
    _tensor -= rhs._tensor;
    return *this;
}

template <typename DT>
Tensor<DT>& Tensor<DT>::operator*=(Tensor<DT> const& rhs) {
    _tensor *= rhs._tensor;
    return *this;
}

template <typename DT>
Tensor<DT>& Tensor<DT>::operator/=(Tensor<DT> const& rhs) {
    _tensor /= rhs._tensor;
    return *this;
}

template <typename DT>
Tensor<DT>& Tensor<DT>::operator*(Tensor<DT> const& rhs) {
    _tensor = xt::linalg::dot(_tensor,rhs._tensor);
    return *this;
}

template <typename U>
Tensor<U> operator+(Tensor<U> lhs, Tensor<U> const& rhs) {
    lhs._tensor += rhs._tensor;
    return lhs;
}
template <typename U>
Tensor<U> operator-(Tensor<U> lhs, Tensor<U> const& rhs) {
    lhs._tensor -= rhs._tensor;
    return lhs;
}
template <typename U>
Tensor<U> operator*(Tensor<U> lhs, Tensor<U> const& rhs) {
    lhs._tensor *= rhs._tensor;
    return lhs;
}
template <typename U>
Tensor<U> operator/(Tensor<U> lhs, Tensor<U> const& rhs) {
    lhs._tensor /= rhs._tensor;
    return lhs;
}

// @brief Returns the shape of the tensor
template <typename DT>
std::vector<size_t> Tensor<DT>::shape() const {
    std::vector<size_t> shape;
    for (size_t i = 0; i < dimension(); ++i) {
        shape.emplace_back(_tensor.shape(i));
    }
    return shape;
}

// reset the tensor axis history to (0, 0), (1, 1), ..., (n-1, n-1)
template <typename DT>
void Tensor<DT>::reset_axis_history() {
    _axis_history.clear();
    for (size_t i = 0; i < _tensor.dimension(); ++i) {
        _axis_history.emplace(i, i);
    }
}

template <typename DT>
size_t Tensor<DT>::get_new_axis_id(size_t const& old_id) {
    if (!_axis_history.contains(old_id)) {
        return SIZE_MAX;
    } else {
        return _axis_history[old_id];
    }
}

//------------------------------
// Tensor Manipulations:
// Member functions
//------------------------------

// Convert the tensor to a matrix, i.e., a 2D tensor according to the two axis lists.
template <typename DT>
Tensor<DT> Tensor<DT>::to_matrix(TensorAxisList const& row_axes, TensorAxisList const& col_axes) {
    using dvlab::utils::int_pow;
    if (!is_partition(*this, row_axes, col_axes)) {
        throw std::invalid_argument("The two axis lists should partition 0~(n-1).");
    }
    Tensor<DT> t = xt::transpose(this->_tensor, concat_axis_list(row_axes, col_axes));
    t._tensor.reshape({int_pow(2, row_axes.size()), int_pow(2, col_axes.size())});
    return t;
}

template <typename U>
Tensor<U> direct_sum(Tensor<U> const& t1, Tensor<U> const& t2) {
    using shape_t = typename xt::xarray<U>::shape_type;
    if (t1.dimension() != 2 || t2.dimension() != 2) {
        throw std::invalid_argument("The two tensors should be 2-dimension.");
    }
    shape_t shape1 = t1._tensor.shape();
    shape_t shape2 = t2._tensor.shape();
    auto tmp1      = xt::hstack(xt::xtuple(t1._tensor, xt::zeros<U>({shape1[0], shape2[1]})));
    auto tmp2      = xt::hstack(xt::xtuple(xt::zeros<U>({shape2[0], shape1[1]}), t2._tensor));

    return xt::vstack(xt::xtuple(tmp1, tmp2));
}

// Rearrange the element of the tensor to a new shape
template <typename DT>
void Tensor<DT>::reshape(TensorShape const& shape) {
    this->_tensor = this->_tensor.reshape(shape);
}

// Rearrange the order of axes
template <typename DT>
Tensor<DT> Tensor<DT>::transpose(TensorAxisList const& perm) const {
    return xt::transpose(_tensor, perm);
}

template <typename DT>
void Tensor<DT>::adjoint() {
    assert(dimension() == 2);
    _tensor = xt::conj(xt::transpose(_tensor, {1, 0}));
}

template <typename DT>
void Tensor<DT>::sqrt() {
    assert(dimension() == 2);
    _tensor = xt::sqrt(_tensor);
}

template <typename DT>
std::vector<std::string> Tensor<DT>::binary_prod(int n){
  std::vector<std::string> bin_list;
  for (int i = 1; i <= n; i++)
  {
    
    for (int j = 0;j < pow(2,i); j++)
    {
      boost::dynamic_bitset<> B(i, j);
      std::string B_string;
      boost::to_string(B,B_string);
      bin_list.push_back(B_string);
      //std::cout<< B_string << std::endl;
    }
  }
  return bin_list;
}

template <typename DT>
std::vector<Tensor<DT>>  Tensor<DT>::create_unitaries(const std::vector<Tensor<DT>>  base, int limit){
    std::vector<Tensor<DT>> gate_list;
    std::vector<std::string> bin_list = binary_prod(limit);
    int a = bin_list.size();
    for (int i = 0; i < a; i++)
    {
        std::complex<double> one(1,0);
        std::complex<double> zero(0,0);
        Tensor<DT> u = {{one,zero},{zero,one}};
       
        std::string bits = bin_list[i];
        int b = bits.length();
        for (int j = 0; j < b; j++)
        {
            char bit = bits[j];
            int index = int(bit) - 48; // Convert char to int (assuming ASCII)
            u *= base[index];
        }
        gate_list.push_back(u);
        //std::cout<< u << std::endl;

    }

    return gate_list;
}

template <typename DT>
void Tensor<DT>::save_unitaries(const std::vector<Tensor<DT>>& unitaries, const std::string& filename){
    std::ofstream out_file(filename, std::ios::binary);
    //out_file.open(filepath);
    if (!out_file.is_open()) {
        spdlog::error("Failed to open file");
        return ;
    }

    int num_matrices = unitaries.size();
    out_file.write(reinterpret_cast<const char*>(&num_matrices), sizeof(int));
    for (auto& u : unitaries){
         for (size_t row = 0; row < u._tensor.shape(0); row++) {
        for (size_t col = 0; col <u. _tensor.shape(1); col++) {
            if (xt::imag(u._tensor(row, col)) >= 0) {
                fmt::print(out_file, "{}{}+{}j", xt::real(u._tensor(row, col)) >= 0 ? " " : "", xt::real(u._tensor(row, col)), abs(xt::imag(u._tensor(row, col))));
            } else {
                fmt::print(out_file, "{}{}{}j", xt::real(u._tensor(row, col)) >= 0 ? " " : "", xt::real(u._tensor(row, col)), xt::imag(u._tensor(row, col)));
            }
            if (col !=u. _tensor.shape(1) - 1) {
                fmt::print(out_file, ", ");
            }
        }
        fmt::println(out_file, "");
      }

    }


    out_file.close();

}

template <typename DT>
Tensor<DT> Tensor<DT>::random_u(double x,double y, double z){
  assert(dimension() == 2);

  std::complex<double> img(0, 1);
  std::complex<double> neg(-1, 0);

  std::complex<double> X(2 * PI * x, 0);
  std::complex<double> Y(2 * PI * y, 0);
  std::complex<double> Z(2 * PI * z, 0);

  Tensor<DT> _X = {{cos(X / 2.0), img*sin(X / 2.0)*neg},{img*sin(X / 2.0*neg), cos(X / 2.0)}};
  Tensor<DT> _Y = {{cos(Y / 2.0), neg*sin(Y / 2.0)},{sin(Y / 2.0), cos(Y / 2.0)}};
  Tensor<DT> _Z = {{pow(M_E,neg*img*Z/2.0) , 0 },{0 , pow(M_E,img*Z/2.0)}};
 
  return _X*_Y*_Z;
}

template <typename DT>
std::vector<std::complex<double>> Tensor<DT>::u_to_bloch(Tensor<DT> u){
  assert(dimension() == 2);
  std::vector<std::complex<double>> axis;
  auto m = u._tensor;
  double angle = (acos((m(0,0) + m(1,1))/2.0)).real();
  double Sin = sin(angle);
  if (Sin < pow(10,-10))
  {
    std::complex<double> nx(0,0);
    std::complex<double> ny(0,0);
    std::complex<double> nz(1,0);
    std::complex<double> angle_2(2*angle,0);
    
    axis.push_back(nx);
    axis.push_back(ny);
    axis.push_back(nz);
    axis.push_back(angle_2);
    /*
    axis[0] = nx;
    axis[1] = ny;
    axis[2] = nz;
    axis[3] = angle_2;
    */
  }
  else
  {
    std::complex<double> j_2(0,2); 
    std::complex<double> nx = (m(0,1) + m(1,0)) / (Sin * j_2);
    std::complex<double> ny = (m(0,1) - m(1,0)) / (Sin * 2);
    std::complex<double> nz = (m(0,0) - m(1,1)) / (Sin * j_2);
    std::complex<double> angle_2(2*angle,0);
    
    axis.push_back(nx);
    axis.push_back(ny);
    axis.push_back(nz);
    axis.push_back(angle_2);
    
    
  }
  return axis;
}

template <typename DT>
std::complex<double> Tensor<DT>::determinant() {
    // TODO - Check correctness and also check the condition dim=2 is needed;
    assert(dimension() == 2);
    return xt::linalg::det(_tensor);
}

template <typename DT>
std::complex<double> Tensor<DT>::trace() {
    // TODO - Check correctness and also check the condition dim=2 is needed;
    assert(dimension() == 2);
    return _tensor(0,0) + _tensor(1,1);
    //return xt::linalg::trace(_tensor);
}

template <typename DT>
auto Tensor<DT>::eigen() {
    // TODO - Check the return type, may be std::pair<value, vector>
    // TODO -
    return xt::linalg::eig(_tensor);
}


template <typename DT>
Tensor<DT>  Tensor<DT>::to_su2(Tensor<DT> &u) {
    std::complex<double> det = u.determinant();
    std::complex<double> one(1,0);
    return (std::sqrt(one / det) * u._tensor);
}


template <typename DT>
void Tensor<DT>::SK_decompose() {
    assert(dimension() == 2);
    // TODO - Move your code here, you may also create new files src/tensor/
   
    /*
    Tensor<DT> a = to_su2(*this);
    std::cout <<"to_su2 the tensor is" <<a << std::endl;
    std::cout <<"trace of the tensor is"<<this->trace()<<std::endl;
    std::tuple b = this->eigen();
    Tensor<DT> value = std::get<0>(b);
    Tensor<DT> vector = std::get<1>(b);
    std::cout <<"eigen value of the tensor is"<<value<<std::endl; //eigen value 
    std::cout <<"eigen vector of the tensor is"<<vector<<std::endl; //eigen vector

    Tensor<DT> u = random_u(0.46181601443868003, 0.9850727657318968, 0.9477936955481501);
    std::cout << u << std::endl;
    Tensor<DT> v = random_u(0.4149234501309683, 0.15937620575851397, 0.10277865702965872);
    std::cout << v << std::endl;
    std::cout <<"trace distance of u,v is"<<trace_distance(u,v)<<std::endl;

    std::vector<std::complex<double>> axis = u_to_bloch(*this);
    std::cout <<"u_to_bloc of u is" << axis[0] <<axis[1] << axis[2] << axis[3] << std::endl;
    */
    std::complex<double> sqrt2(std::sqrt(2),0); 
    std::complex<double> t(0,PI/4); 
    Tensor<DT> H = {{1./sqrt2,1./sqrt2},{1./sqrt2,-1./sqrt2}};
    Tensor<DT> T = {{1,0},{0,pow(M_E,t)}};
    std::vector<Tensor<DT>> base = {to_su2(H), to_su2(T)};

    std::cout<< base[0] << base[1] << std::endl;
    //generate gate list
    for(int i = 1; i < 16; i++){
        std::vector<Tensor<DT>> unitaries = create_unitaries(base, i);
        save_unitaries(unitaries, "gate_list_" + std::to_string(i) + ".dat");
    }
    
}



template <typename DT>
bool Tensor<DT>::tensor_write(std::string const& filepath) {
    std::ofstream out_file;
    out_file.open(filepath);
    if (!out_file.is_open()) {
        spdlog::error("Failed to open file");
        return false;
    }
    for (size_t row = 0; row < _tensor.shape(0); row++) {
        for (size_t col = 0; col < _tensor.shape(1); col++) {
            if (xt::imag(this->_tensor(row, col)) >= 0) {
                fmt::print(out_file, "{}{}+{}j", xt::real(_tensor(row, col)) >= 0 ? " " : "", xt::real(_tensor(row, col)), abs(xt::imag(_tensor(row, col))));
            } else {
                fmt::print(out_file, "{}{}{}j", xt::real(_tensor(row, col)) >= 0 ? " " : "", xt::real(_tensor(row, col)), xt::imag(_tensor(row, col)));
            }
            if (col != _tensor.shape(1) - 1) {
                fmt::print(out_file, ", ");
            }
        }
        fmt::println(out_file, "");
    }
    out_file.close();
    return true;
}

template <typename DT>
bool Tensor<DT>::tensor_read(std::string const& filepath) {
    std::ifstream in_file;
    in_file.open(filepath);
    if (!in_file.is_open()) {
        spdlog::error("Failed to open file");
        return false;
    }
    // read csv with complex number
    std::vector<std::complex<double>> data;
    std::string line, word;
    while (std::getline(in_file, line)) {
        std::stringstream ss(line);
        while (std::getline(ss, word, ',')) {
            std::stringstream ww(word);

            double real = 0.0, imag = 0.0;
            char plus = ' ', i = ' ';
            ww >> real >> plus >> imag >> i;
            if (plus == '-') imag = -imag;
            if (plus == 'j') {
                imag = real;
                real = 0;
            }
            data.push_back({real, imag});
        }
    }
    if (std::floor(std::sqrt(data.size())) != std::sqrt(data.size())) {
        spdlog::error("The number of elements in the tensor is not a square number");
        return false;
    }
    const TensorShape shape = {static_cast<size_t>(std::sqrt(data.size())), static_cast<size_t>(std::sqrt(data.size()))};
    this->_tensor           = xt::adapt(data, shape);
    return true;
}

//------------------------------
// Tensor Manipulations:
// Friend functions
//------------------------------

// Calculate the inner products between two tensors
template <typename U>
double inner_product(Tensor<U> const& t1, Tensor<U> const& t2) {
    if (t1.shape() != t2.shape()) {
        throw std::invalid_argument("The two tensors should have the same shape");
    }
    return xt::abs(xt::sum(xt::conj(t1._tensor) * t2._tensor))();
}
// Calculate the cosine similarity of two tensors
template <typename U>
double cosine_similarity(Tensor<U> const& t1, Tensor<U> const& t2) {
    return inner_product(t1, t2) / std::sqrt(inner_product(t1, t1) * inner_product(t2, t2));
}
// Calculate the trace distance of two tensors
template <typename DT>
double Tensor<DT>::trace_distance(Tensor<DT>& t1, Tensor<DT>& t2) {
    // TODO
    Tensor<DT> t1_t2 = t1 - t2;
    t1_t2.adjoint();
    t1_t2.sqrt();
    std::complex<double> t(0.5,0);
    return (0.5 * t1_t2.trace()).real();
}

// tensor-dot two tensors
// dots the two tensors along the axes in ax1 and ax2
template <typename U>
Tensor<U> tensordot(Tensor<U> const& t1, Tensor<U> const& t2,
                    TensorAxisList const& ax1 = {}, TensorAxisList const& ax2 = {}) {
    if (ax1.size() != ax2.size()) {
        throw std::invalid_argument("The two index orders should contain the same number of indices.");
    }
    Tensor<U> t        = xt::linalg::tensordot(t1._tensor, t2._tensor, ax1, ax2);
    size_t tmp_axis_id = 0;
    t._axis_history.clear();
    for (size_t i = 0; i < t1._tensor.dimension(); ++i) {
        if (std::find(ax1.begin(), ax1.end(), i) != ax1.end()) continue;
        t._axis_history.emplace(i, tmp_axis_id);
        ++tmp_axis_id;
    }
    for (size_t i = 0; i < t2._tensor.dimension(); ++i) {
        if (std::find(ax2.begin(), ax2.end(), i) != ax2.end()) continue;
        t._axis_history.emplace(i + t1._tensor.dimension(), tmp_axis_id);
        ++tmp_axis_id;
    }
    return t;
}

// Calculate tensor powers
template <typename U>
Tensor<U> tensor_product_pow(Tensor<U> const& t, size_t n) {
    if (n == 0) return xt::ones<U>(TensorShape(0, 2));
    if (n == 1) return t;
    auto tmp = tensor_product_pow(t, n / 2);
    if (n % 2 == 0)
        return tensordot(tmp, tmp);
    else
        return tensordot(t, tensordot(tmp, tmp));
}

// Returns true if two axis lists form a partition spanning axis 0 to n-1, where n is the dimension of the tensor.
template <typename U>
bool is_partition(Tensor<U> const& t, TensorAxisList const& axes1, TensorAxisList const& axes2) {
    if (!is_disjoint(axes1, axes2)) return false;
    if (axes1.size() + axes2.size() != t._tensor.dimension()) return false;
    return true;
}

template <typename U>
Tensor<U> tensor_multiply(Tensor<U> const& t1, Tensor<U> const& t2) {
    // fmt::println("in tensor multiply function");
    return tensordot(t1, t2, {1}, {0});
}

}  // namespace qsyn::tensor

template <typename DT>
struct fmt::formatter<qsyn::tensor::Tensor<DT>> : fmt::ostream_formatter {};
