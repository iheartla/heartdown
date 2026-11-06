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
#include <unsupported/Eigen/MatrixFunctions>

struct pipeline {
    using DT = double;
    double f_x;
    double f_y;
    double c_x;
    double c_y;
    double r;
    std::function<Eigen::Matrix<double, 2, 1>(Eigen::Matrix<double, 2, 1>)> φ_circumflex_accent_1_d;
    std::function<Eigen::Matrix<double, 2, 1>(Eigen::Matrix<double, 2, 1>)> φ_d;
    Eigen::Matrix<double, 2, 1> x_i;
    Eigen::Matrix<double, 2, 1> x_j;
    double α_i;
    double α_j;
    double y;
    double X;
    double Y;
    double Z;
    double x;
    Eigen::Matrix<double, 1, 3> textbfX;
    Eigen::Matrix<double, 3, 3> K;
    Eigen::Matrix<double, 3, 1> textbft;
    double A;
    double B;
    double D;
    double γ;
    double C;
    double ϕ;
    double α_1;
    double α_2;

    pipeline(const double & f_x, const double & f_y, const double & c_x, const double & c_y, const double & r, const std::function<Eigen::Matrix<double, 2, 1>(Eigen::Matrix<double, 2, 1>)> & φ_circumflex_accent_1_d, const std::function<Eigen::Matrix<double, 2, 1>(Eigen::Matrix<double, 2, 1>)> & φ_d, const Eigen::Matrix<double, 2, 1> & x_i, const Eigen::Matrix<double, 2, 1> & x_j, const double & α_i, const double & α_j, const double & y, const double & X, const double & Y, const double & Z, const double & x)
    {
        this->f_x = f_x;
        this->f_y = f_y;
        this->c_x = c_x;
        this->c_y = c_y;
        this->r = r;
        this->φ_circumflex_accent_1_d = φ_circumflex_accent_1_d;
        this->φ_d = φ_d;
        this->x_i = x_i;
        this->x_j = x_j;
        this->α_i = α_i;
        this->α_j = α_j;
        this->y = y;
        this->X = X;
        this->Y = Y;
        this->Z = Z;
        this->x = x;
        // `$\textbf{X}$` = (X,Y,Z)^T
        this->textbfX = [&](){ Eigen::Matrix<double, 3, 1> v(3); v << this->X, this->Y, this->Z; return v; }().transpose();
        // K = [`$f_x$` 0 `$c_x$`
        //       0   `$f_y$` `$c_y$`
        //       0      0    1]
        this->K = [&](){ Eigen::MatrixXd m(3, 3); m << this->f_x, 0, this->c_x, 0, this->f_y, this->c_y, 0, 0, 1; return m; }();
        // `$\textbf{t}$` = [0;0;-r]
        this->textbft = [&](){ Eigen::MatrixXd m(3, 1); m << 0, 0, -this->r; return m; }();
        // A = X ⋅ `$f_x$` - Z⋅(x - `$c_x$` )
        this->A = this->X * this->f_x - this->Z * (this->x - this->c_x);
        // B = Z⋅`$f_x$` + X⋅(x -`$c_x$` )
        this->B = this->Z * this->f_x + this->X * (this->x - this->c_x);
        // D = √(A^2 +B^2)
        this->D = sqrt((pow(this->A, 2) + pow(this->B, 2)));
        // γ = arctan(B/A)
        this->γ = atan(this->B / this->A);
        // C = -r⋅(x -`$c_x$` )
        this->C = -(this->r * (this->x - this->c_x));
        // ϕ = arcsin(C/D)
        this->ϕ = asin(this->C / this->D);
        // `$α_1$` = ϕ - γ
        this->α_1 = this->ϕ - this->γ;
        // `$α_2$` = π - ϕ - γ
        this->α_2 = M_PI - this->ϕ - this->γ;
    }

    Eigen::Matrix<double, 3, 3> R(const double & α)
    {
        return [&](){ Eigen::MatrixXd m(3, 3); m << -sin(α), 0, -cos(α), 0, 1, 0, cos(α), 0, -sin(α); return m; }();
    }

    Eigen::Matrix<double, 3, 4> P(const double & α)
    {
        return this->K * [&](){ Eigen::MatrixXd m(3, 4); m << this->R(α), this->textbft; return m; }();
    }

    double t(const double & α)
    {
        return (α - this->α_i) / (this->α_j - this->α_i);
    }

    Eigen::Matrix<double, 2, 1> textbfx(const double & α)
    {
        return this->φ_circumflex_accent_1_d((1 - this->t(α)) * this->φ_d(this->x_i) + this->t(α) * this->φ_d(this->x_j));
    }

    double ω(const double & x)
    {
        return atan((x - this->c_x) / this->f_x);
    }

    double s(const double & x)
    {
        return (this->y - this->c_y) * cos(this->ω(x));
    }

    Eigen::Matrix<double, 2, 1> φ(const double & x)
    {
        return [&](){ Eigen::Matrix<double, 2, 1> v(2); v << this->ω(x), this->s(x); return v; }();
    }
};

