#include <nanobind/nanobind.h>
#include <nanobind/ndarray.h>
#include <compute/linalg.hpp>
#include <compute/core/compute_policy.hpp>
#include <stdexcept>
#include <string>

namespace nb = nanobind;
using namespace compute::linalg;
using compute::core::ComputePolicy;

namespace {

[[noreturn]] void throw_on_error(const compute::core::ComputeError& err)
{
    throw std::runtime_error(std::string(err.msg));
}

template<typename T>
void require_fortran_matrix(const nb::ndarray<T, nb::ndim<2>>& arr, const char* name)
{
    if (arr.ndim() != 2)
        throw std::runtime_error(std::string(name) + " must be a 2-D array");
    if (arr.stride(0) != 1)
        throw std::runtime_error(std::string(name)
                                 + " must be column-major (numpy order='F')");
}

template<typename T>
void require_vector(const nb::ndarray<T, nb::ndim<1>>& arr, const char* name)
{
    if (arr.ndim() != 1)
        throw std::runtime_error(std::string(name) + " must be a 1-D array");
}

template<typename TA, typename TB>
void require_same_shape_2d(const nb::ndarray<TA, nb::ndim<2>>& a,
                           const nb::ndarray<TB, nb::ndim<2>>& b,
                           const char* b_name)
{
    if (a.shape(0) != b.shape(0) || a.shape(1) != b.shape(1))
        throw std::runtime_error(std::string("shape mismatch between operands and ") + b_name);
}

const_matrix_view as_const_matrix(const nb::ndarray<const double, nb::ndim<2>>& arr)
{
    require_fortran_matrix(arr, "input");
    auto view = const_matrix_view::create(
        {arr.data(), static_cast<std::size_t>(arr.size())},
        static_cast<std::size_t>(arr.shape(0)),
        static_cast<std::size_t>(arr.shape(1)));
    if (!view) throw_on_error(view.error());
    return *view;
}

matrix_view as_matrix(const nb::ndarray<double, nb::ndim<2>>& arr)
{
    require_fortran_matrix(arr, "output");
    auto view = matrix_view::create(
        {arr.data(), static_cast<std::size_t>(arr.size())},
        static_cast<std::size_t>(arr.shape(0)),
        static_cast<std::size_t>(arr.shape(1)));
    if (!view) throw_on_error(view.error());
    return *view;
}

void py_add(nb::ndarray<const double, nb::ndim<2>> a,
            nb::ndarray<const double, nb::ndim<2>> b,
            nb::ndarray<double, nb::ndim<2>> c)
{
    require_same_shape_2d(a, b, "b");
    require_same_shape_2d(a, c, "c");
    if (auto r = add(as_const_matrix(a), as_const_matrix(b), as_matrix(c)); !r)
        throw_on_error(r.error());
}

void py_sub(nb::ndarray<const double, nb::ndim<2>> a,
            nb::ndarray<const double, nb::ndim<2>> b,
            nb::ndarray<double, nb::ndim<2>> c)
{
    require_same_shape_2d(a, b, "b");
    require_same_shape_2d(a, c, "c");
    if (auto r = sub(as_const_matrix(a), as_const_matrix(b), as_matrix(c)); !r)
        throw_on_error(r.error());
}

void py_scale(double alpha,
              nb::ndarray<const double, nb::ndim<2>> a,
              nb::ndarray<double, nb::ndim<2>> b)
{
    require_same_shape_2d(a, b, "output");
    if (auto r = scale(alpha, as_const_matrix(a), as_matrix(b)); !r)
        throw_on_error(r.error());
}

void py_hadamard(nb::ndarray<const double, nb::ndim<2>> a,
                 nb::ndarray<const double, nb::ndim<2>> b,
                 nb::ndarray<double, nb::ndim<2>> c)
{
    require_same_shape_2d(a, b, "b");
    require_same_shape_2d(a, c, "c");
    if (auto r = hadamard(as_const_matrix(a), as_const_matrix(b), as_matrix(c)); !r)
        throw_on_error(r.error());
}

void py_transpose(nb::ndarray<const double, nb::ndim<2>> a,
                  nb::ndarray<double, nb::ndim<2>> b)
{
    require_fortran_matrix(a, "input");
    require_fortran_matrix(b, "output");
    if (a.shape(0) != b.shape(1) || a.shape(1) != b.shape(0))
        throw std::runtime_error("output shape must be the transpose of input");
    if (auto r = transpose(as_const_matrix(a), as_matrix(b)); !r)
        throw_on_error(r.error());
}

double py_dot(nb::ndarray<const double, nb::ndim<1>> u,
              nb::ndarray<const double, nb::ndim<1>> v)
{
    require_vector(u, "u");
    require_vector(v, "v");
    if (u.shape(0) != v.shape(0))
        throw std::runtime_error("vector lengths must match");
    auto r = dot({u.data(), static_cast<std::size_t>(u.size())},
                 {v.data(), static_cast<std::size_t>(v.size())});
    if (!r) throw_on_error(r.error());
    return *r;
}

void py_gemv(nb::ndarray<const double, nb::ndim<2>> a,
             nb::ndarray<const double, nb::ndim<1>> x,
             nb::ndarray<double, nb::ndim<1>> y)
{
    require_fortran_matrix(a, "A");
    require_vector(x, "x");
    require_vector(y, "y");
    if (x.shape(0) != a.shape(1))
        throw std::runtime_error("x length must match column count of A");
    if (y.shape(0) != a.shape(0))
        throw std::runtime_error("y length must match row count of A");
    if (auto r = gemv(as_const_matrix(a),
                       {x.data(), static_cast<std::size_t>(x.size())},
                       {y.data(), static_cast<std::size_t>(y.size())});
        !r)
        throw_on_error(r.error());
}

void py_gemm(nb::ndarray<const double, nb::ndim<2>> a,
             nb::ndarray<const double, nb::ndim<2>> b,
             nb::ndarray<double, nb::ndim<2>> c)
{
    require_fortran_matrix(a, "A");
    require_fortran_matrix(b, "B");
    require_fortran_matrix(c, "C");
    if (a.shape(1) != b.shape(0))
        throw std::runtime_error("inner dimensions of A and B must match");
    if (c.shape(0) != a.shape(0) || c.shape(1) != b.shape(1))
        throw std::runtime_error("C shape must be (A.rows, B.cols)");
    if (auto r = gemm(as_const_matrix(a), as_const_matrix(b), as_matrix(c)); !r)
        throw_on_error(r.error());
}

double py_trace(nb::ndarray<const double, nb::ndim<2>> a)
{
    auto r = trace(as_const_matrix(a));
    if (!r) throw_on_error(r.error());
    return *r;
}

double py_norm_fro(nb::ndarray<const double, nb::ndim<2>> a)
{
    auto r = norm_fro(as_const_matrix(a));
    if (!r) throw_on_error(r.error());
    return *r;
}

double py_norm_inf(nb::ndarray<const double, nb::ndim<2>> a)
{
    auto r = norm_inf(as_const_matrix(a));
    if (!r) throw_on_error(r.error());
    return *r;
}

} // namespace

NB_MODULE(_compute_linalg, m)
{
    m.doc() = "compute engine — linear algebra (CPU + optional GPU dispatch)";

    m.def("add", &py_add, nb::arg("a"), nb::arg("b"), nb::arg("c"),
          "Out-of-place matrix addition: c = a + b");
    m.def("sub", &py_sub, nb::arg("a"), nb::arg("b"), nb::arg("c"),
          "Out-of-place matrix subtraction: c = a - b");
    m.def("scale", &py_scale, nb::arg("alpha"), nb::arg("a"), nb::arg("b"),
          "Scalar multiply: b = alpha * a");
    m.def("hadamard", &py_hadamard, nb::arg("a"), nb::arg("b"), nb::arg("c"),
          "Hadamard product: c = a ⊙ b");
    m.def("transpose", &py_transpose, nb::arg("a"), nb::arg("b"),
          "Out-of-place transpose: b = a.T");
    m.def("dot", &py_dot, nb::arg("u"), nb::arg("v"), "Vector dot product");
    m.def("gemv", &py_gemv, nb::arg("a"), nb::arg("x"), nb::arg("y"),
          "Matrix-vector multiply: y = a @ x");
    m.def("gemm", &py_gemm, nb::arg("a"), nb::arg("b"), nb::arg("c"),
          "Matrix multiply: c = a @ b");
    m.def("trace", &py_trace, nb::arg("a"), "Matrix trace");
    m.def("norm_fro", &py_norm_fro, nb::arg("a"), "Frobenius norm");
    m.def("norm_inf", &py_norm_inf, nb::arg("a"), "Infinity operator norm");
}
