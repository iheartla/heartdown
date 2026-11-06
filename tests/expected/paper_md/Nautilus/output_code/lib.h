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

struct nautilus {
    using DT = double;
    Eigen::VectorXd x;
    double λ;
    std::vector<Eigen::Matrix<double, 3, 3>> v;
    long n;
    long v_n;
    Eigen::Matrix<double, 3, 3> P;
    Eigen::Matrix<double, 3, 3> S;
    Eigen::Matrix<double, 3, 3> H;

    nautilus(const Eigen::VectorXd & x, const double & λ, const std::vector<Eigen::Matrix<double, 3, 3>> & v)
    {
        this->x = x;
        this->λ = λ;
        this->v = v;
        this->n = x.rows();
        this->v_n = v.size();
        assert( x.rows() == this->n );
        // P = [1 x₁ 0
        //      0 x₂ 0
        //      x₃ x₄ 1]
        this->P = [&](){ Eigen::MatrixXd m(3, 3); m << 1, this->x(0), 0, 0, this->x(1), 0, this->x(2), this->x(3), 1; return m; }();
        // S = [x₅ x₆ x₇
        //      x₈ x₉ x₁₀
        //      0 0 1]
        this->S = [&](){ Eigen::MatrixXd m(3, 3); m << this->x(4), this->x(5), this->x(6), this->x(7), this->x(8), this->x(9), 0, 0, 1; return m; }();
        // H = PSP⁻¹
        this->H = (this->P * this->S) * (this->P).inverse();
    }

    double E(const Eigen::VectorXd & x)
    {
        return this->λ * pow(([&](){ Eigen::Matrix<double, 2, 1> v(2); v << x(4), x(7); return v; }() - [&](){ Eigen::Matrix<double, 2, 1> v(2); v << x(8), x(5); return v; }()).norm(), 2) + [&](){ double sum = 0.0; for(int i = 1; i <= this->v_n; i++){ sum += pow((this->H * this->v.at(i-1) - (this->P * this->S) * ((this->P).eval()).colPivHouseholderQr().solve((this->v.at(i-1)).eval())).norm(), 2); } return sum; }();
    }
};

