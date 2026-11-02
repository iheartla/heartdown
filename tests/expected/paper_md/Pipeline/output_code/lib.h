#pragma once
#include <Eigen/Core>
#include <Eigen/QR>
#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <iostream>
#include <set>
#include <algorithm>
#include "type_helper.h"

struct pipeline {
    using DT = double;
    using MatrixD = Eigen::MatrixXd;
    using VectorD = Eigen::VectorXd;
    Eigen::Matrix<DT, 1, 3> textbfX;
    DT ϕ;
    DT D;
    DT γ;
    Eigen::Matrix<DT, 3, 3> K;
    Eigen::Matrix<DT, 3, 1> textbft;
    DT A;
    DT B;
    DT C;
    DT α_1;
    DT α_2;
    double α_i;
    double α_j;
    std::function<Eigen::Matrix<double, 2, 1>(Eigen::Matrix<double, 2, 1>)> φ_circumflex_accent_1_d;
    std::function<Eigen::Matrix<double, 2, 1>(Eigen::Matrix<double, 2, 1>)> φ_d;
    Eigen::Matrix<double, 2, 1> x_i;
    Eigen::Matrix<double, 2, 1> x_j;
    double x;
    double c_x;
    double f_x;
    double y;
    double c_y;
    template<typename REAL>
    Eigen::Matrix<REAL, 3, 3> R(
        const REAL & α)
    {
        Eigen::Matrix<REAL, 3, 3> R_0;
        R_0 << -sin(α), 0, -cos(α),
        0, 1, 0,
        cos(α), 0, -sin(α);
        return R_0;    
    }
    template<typename REAL>
    Eigen::Matrix<REAL, 3, 4> P(
        const REAL & α)
    {
        Eigen::Matrix<REAL, 3, 4> P_0;
        P_0 << R(α), this->textbft;
        return this->K * P_0;    
    }
    template<typename REAL>
    REAL t(
        const REAL & α)
    {
        return (α - this->α_i) / REAL((this->α_j - this->α_i));    
    }
    template<typename REAL>
    Eigen::Matrix<REAL, 2, 1> textbfx(
        const REAL & α)
    {
        return this->φ_circumflex_accent_1_d(((1 - t(α)) * this->φ_d((this->x_i).eval()) + t(α) * this->φ_d((this->x_j).eval())).eval());    
    }
    template<typename REAL>
    REAL ω(
        const REAL & x)
    {
        return atan((x - this->c_x) / REAL(this->f_x));    
    }
    template<typename REAL>
    REAL s(
        const REAL & x)
    {
        return (this->y - this->c_y) * cos(ω(x));    
    }
    template<typename REAL>
    Eigen::Matrix<REAL, 2, 1> φ(
        const REAL & x)
    {
        Eigen::Matrix<REAL, 2, 1> φ_0;
        φ_0 << ω(x), s(x);
        return φ_0;    
    }
    pipeline(
        const double & f_x,
        const double & f_y,
        const double & c_x,
        const double & c_y,
        const double & r,
        const std::function<Eigen::Matrix<double, 2, 1>(Eigen::Matrix<double, 2, 1>)> & φ_circumflex_accent_1_d,
        const std::function<Eigen::Matrix<double, 2, 1>(Eigen::Matrix<double, 2, 1>)> & φ_d,
        const Eigen::Matrix<double, 2, 1> & x_i,
        const Eigen::Matrix<double, 2, 1> & x_j,
        const double & α_i,
        const double & α_j,
        const double & y,
        const double & X,
        const double & Y,
        const double & Z,
        const double & x)
    {
    
        this->α_i = α_i;
        this->α_j = α_j;
        this->φ_circumflex_accent_1_d = φ_circumflex_accent_1_d;
        this->φ_d = φ_d;
        this->x_i = x_i;
        this->x_j = x_j;
        this->x = x;
        this->c_x = c_x;
        this->f_x = f_x;
        this->y = y;
        this->c_y = c_y;
        // `$\textbf{X}$` = (X,Y,Z)^T
        Eigen::Matrix<DT, 3, 1> textbfX_0;
        textbfX_0 << X, Y, Z;
        textbfX = textbfX_0.transpose();
        // K = [`$f_x$` 0 `$c_x$`
        //       0   `$f_y$` `$c_y$`
        //       0      0    1]
        Eigen::Matrix<DT, 3, 3> K_0;
        K_0 << f_x, 0, c_x,
        0, f_y, c_y,
        0, 0, 1;
        K = K_0;
        // `$\textbf{t}$` = [0;0;-r]
        Eigen::Matrix<DT, 3, 1> textbft_0;
        textbft_0 << 0,
        0,
        -r;
        textbft = textbft_0;
        // A = X ⋅ `$f_x$` - Z⋅(x - `$c_x$` )
        A = X * f_x - Z * (x - c_x);
        // B = Z⋅`$f_x$` + X⋅(x -`$c_x$` )
        B = Z * f_x + X * (x - c_x);
        // D = √(A^2 +B^2)
        D = sqrt((pow(A, 2) + pow(B, 2)));
        // γ = arctan(B/A)
        γ = atan(B / DT(A));
        // C = -r⋅(x -`$c_x$` )
        C = -r * (x - c_x);
        // ϕ = arcsin(C/D)
        ϕ = asin(C / DT(D));
        // `$α_1$` = ϕ - γ
        α_1 = ϕ - γ;
        // `$α_2$` = π - ϕ - γ
        α_2 = M_PI - ϕ - γ;
    }
};

