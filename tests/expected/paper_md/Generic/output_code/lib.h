#pragma once
#include <Eigen/Core>
#include <Eigen/QR>
#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <Eigen/SVD>
#include <iostream>
#include <set>
#include <vector>
#include <tuple>
#include <functional>
#include <algorithm>
#include <cmath>
#include <stdexcept>
#ifndef IHEARTLA_SORTED_SET
#define IHEARTLA_SORTED_SET
// sets are sorted vectors without duplicates (defined before the last #include:
// code that wraps the generated code in a namespace starts it after that)
template<typename T> std::vector<T> sorted_set(std::vector<T> v) { std::sort(v.begin(), v.end()); v.erase(std::unique(v.begin(), v.end()), v.end()); return v; }
// (vectors in lexicographic order)
template<typename S, int R, int C, int O, int MR, int MC> std::vector<Eigen::Matrix<S, R, C, O, MR, MC>> sorted_set(std::vector<Eigen::Matrix<S, R, C, O, MR, MC>> v) {
using M = Eigen::Matrix<S, R, C, O, MR, MC>;
std::sort(v.begin(), v.end(), [](const M & a, const M & b){ return std::lexicographical_compare(a.data(), a.data() + a.size(), b.data(), b.data() + b.size()); });
v.erase(std::unique(v.begin(), v.end(), [](const M & a, const M & b){ return a.size() == b.size() && (a.array() == b.array()).all(); }), v.end());
return v;
}
#endif
#include <unsupported/Eigen/MatrixFunctions>

struct Generic {
    using DT = double;
    Eigen::Matrix<double, 2, 2> J;
    double x;
    double y;
    double u;
    double v;
    Eigen::Matrix<double, 2, 1> x_p;
    Eigen::Matrix<double, 2, 1> v_p;
    Eigen::Matrix<double, 2, 6> M;

    Generic(const Eigen::Matrix<double, 2, 2> & J, const double & x, const double & y, const double & u, const double & v)
    {
        this->J = J;
        this->x = x;
        this->y = y;
        this->u = u;
        this->v = v;
        // `$x_p$` = (-y, x)
        this->x_p = [&](){ Eigen::Matrix<double, 2, 1> v(2); v << -this->y, this->x; return v; }();
        // `$v_p$` = (-v, u)
        this->v_p = [&](){ Eigen::Matrix<double, 2, 1> v(2); v << -this->v, this->u; return v; }();
        // M=[-J`$x_p$`+`$v_p$`  J  `$x_p$`  I_2]
        this->M = [&](){ Eigen::MatrixXd m(2, 6); m << -(this->J * this->x_p) + this->v_p, this->J, this->x_p, Eigen::Matrix<double, 2, 2>::Identity(2, 2); return m; }();
    }
};

