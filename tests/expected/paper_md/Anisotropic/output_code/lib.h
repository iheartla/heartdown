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

struct Anisotropic {
    using DT = double;
    Eigen::Matrix<double, 3, 1> a;
    Eigen::Matrix<double, 3, 3> C;
    Eigen::Matrix<double, 3, 3> F;
    Eigen::Matrix<double, 3, 3> A;
    double lambda_0_comma_1_comma_2;
    double I_5;
    Eigen::Matrix<double, 3, 3> fracdeeI5deeF;
    Eigen::Matrix<double, 9, 9> fracdee2I5deef2;
    Eigen::Matrix<double, 3, 3> mathbfQ_0;
    Eigen::Matrix<double, 3, 3> mathbfQ_1;
    Eigen::Matrix<double, 3, 3> mathbfQ_2;

    Anisotropic(const Eigen::Matrix<double, 3, 1> & a, const Eigen::Matrix<double, 3, 3> & C, const Eigen::Matrix<double, 3, 3> & F)
    {
        this->a = a;
        this->C = C;
        this->F = F;
        // A = a a^T
        this->A = this->a * this->a.transpose();
        // `$\lambda_{0,1,2}$`=2||a||_2^2
        this->lambda_0_comma_1_comma_2 = 2 * pow((this->a).norm(), 2);
        // `$I_5$` = tr(CA)
        this->I_5 = (this->C * this->A).trace();
        // `$\frac{∂I₅}{∂F}$` = 2FA
        this->fracdeeI5deeF = (2 * this->F) * this->A;
        // `$\frac{∂²I₅}{∂f²}$` = 2[A₁,₁I₃  A₁,₂I₃  A₁,₃I₃
        //                A₂,₁I₃  A₂,₂I₃  A₂,₃I₃
        //                A₃,₁I₃  A₃,₂I₃  A₃,₃I₃]
        this->fracdee2I5deef2 = 2 * [&](){ Eigen::MatrixXd m(9, 9); m << this->A(0, 0) * Eigen::Matrix<double, 3, 3>::Identity(3, 3), this->A(0, 1) * Eigen::Matrix<double, 3, 3>::Identity(3, 3), this->A(0, 2) * Eigen::Matrix<double, 3, 3>::Identity(3, 3), this->A(1, 0) * Eigen::Matrix<double, 3, 3>::Identity(3, 3), this->A(1, 1) * Eigen::Matrix<double, 3, 3>::Identity(3, 3), this->A(1, 2) * Eigen::Matrix<double, 3, 3>::Identity(3, 3), this->A(2, 0) * Eigen::Matrix<double, 3, 3>::Identity(3, 3), this->A(2, 1) * Eigen::Matrix<double, 3, 3>::Identity(3, 3), this->A(2, 2) * Eigen::Matrix<double, 3, 3>::Identity(3, 3); return m; }();
        // `$\mathbf{Q}_{0}$` = [a^T
        //                0
        //                0]
        this->mathbfQ_0 = [&](){ Eigen::MatrixXd m(3, 3); m << this->a.transpose(), Eigen::Matrix<double, 1, 3>::Zero(1, 3), Eigen::Matrix<double, 1, 3>::Zero(1, 3); return m; }();
        // `$\mathbf{Q}_{1}$` = [0
        //                     a^T
        //                0]
        this->mathbfQ_1 = [&](){ Eigen::MatrixXd m(3, 3); m << Eigen::Matrix<double, 1, 3>::Zero(1, 3), this->a.transpose(), Eigen::Matrix<double, 1, 3>::Zero(1, 3); return m; }();
        // `$\mathbf{Q}_{2}$` = [ 0
        //                0
        //                a^T]
        this->mathbfQ_2 = [&](){ Eigen::MatrixXd m(3, 3); m << Eigen::Matrix<double, 1, 3>::Zero(1, 3), Eigen::Matrix<double, 1, 3>::Zero(1, 3), this->a.transpose(); return m; }();
    }
};

struct Anisotropic2D {
    using DT = double;
    Eigen::Matrix<double, 2, 1> a;
    Eigen::Matrix<double, 2, 2> A;
    double lambda_0_comma_1;
    Eigen::Matrix<double, 4, 4> fracdee2I5deef2;
    Eigen::Matrix<double, 2, 2> mathbfQ_0;
    Eigen::Matrix<double, 2, 2> mathbfQ_1;

    Anisotropic2D(const Eigen::Matrix<double, 2, 1> & a, const Eigen::Matrix<double, 2, 2> & A)
    {
        this->a = a;
        this->A = A;
        // `$\lambda_{0,1}$`=2||a||_2^2
        this->lambda_0_comma_1 = 2 * pow((this->a).norm(), 2);
        // `$\frac{∂²I₅}{∂f²}$` = 2[A₁,₁I_2  A₁,₂I_2
        //                A₂,₁I_2  A₂,₂I_2]
        this->fracdee2I5deef2 = 2 * [&](){ Eigen::MatrixXd m(4, 4); m << this->A(0, 0) * Eigen::Matrix<double, 2, 2>::Identity(2, 2), this->A(0, 1) * Eigen::Matrix<double, 2, 2>::Identity(2, 2), this->A(1, 0) * Eigen::Matrix<double, 2, 2>::Identity(2, 2), this->A(1, 1) * Eigen::Matrix<double, 2, 2>::Identity(2, 2); return m; }();
        // `$\mathbf{Q}_{0}$` = [a^T
        //                0]
        this->mathbfQ_0 = [&](){ Eigen::MatrixXd m(2, 2); m << this->a.transpose(), Eigen::Matrix<double, 1, 2>::Zero(1, 2); return m; }();
        // `$\mathbf{Q}_{1}$` = [0
        //                     a^T]
        this->mathbfQ_1 = [&](){ Eigen::MatrixXd m(2, 2); m << Eigen::Matrix<double, 1, 2>::Zero(1, 2), this->a.transpose(); return m; }();
    }
};

