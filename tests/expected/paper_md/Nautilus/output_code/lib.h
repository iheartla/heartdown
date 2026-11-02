#pragma once
#include <Eigen/Core>
#include <Eigen/QR>
#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <iostream>
#include <set>
#include <algorithm>
#include "type_helper.h"

struct nautilus {
    using DT = double;
    using MatrixD = Eigen::MatrixXd;
    using VectorD = Eigen::VectorXd;
    Eigen::Matrix<DT, 3, 3> H;
    Eigen::Matrix<DT, 3, 3> P;
    Eigen::Matrix<DT, 3, 3> S;
    Eigen::VectorXd x;
    double λ;
    std::vector<Eigen::Matrix<double, 3, 3>> v;
    template<typename REAL>
    REAL E(
        const Eigen::Matrix<REAL, Eigen::Dynamic, 1> & x)
    {
        const long n = x.size();
        Eigen::Matrix<REAL, 2, 1> E_0;
        E_0 << x[5-1], x[8-1];
        Eigen::Matrix<REAL, 2, 1> E_1;
        E_1 << x[9-1], x[6-1];
        REAL sum_0 = 0;
        for(int i=1; i<=v.size(); i++){
            sum_0 += pow((this->H * this->v.at(i-1) - this->P * this->S * (to_double(this->P)).colPivHouseholderQr().solve(to_double(this->v.at(i-1)))).norm(), 2);
        }
        return this->λ * pow((E_0 - E_1).template lpNorm<2>(), 2) + sum_0;    
    }
    nautilus(
        const Eigen::VectorXd & x,
        const double & λ,
        const std::vector<Eigen::Matrix<double, 3, 3>> & v)
    {
        const long n = x.size();
        const long dim_0 = v.size();
        this->x = x;
        this->λ = λ;
        this->v = v;
        // P = [1 x₁ 0
        //      0 x₂ 0
        //      x₃ x₄ 1]
        Eigen::Matrix<DT, 3, 3> P_0;
        P_0 << 1, x[1-1], 0,
        0, x[2-1], 0,
        x[3-1], x[4-1], 1;
        P = P_0;
        // S = [x₅ x₆ x₇
        //      x₈ x₉ x₁₀
        //      0 0 1]
        Eigen::Matrix<DT, 3, 3> S_0;
        S_0 << x[5-1], x[6-1], x[7-1],
        x[8-1], x[9-1], x[10-1],
        0, 0, 1;
        S = S_0;
        // H = PSP⁻¹
        H = P * S * P.inverse();
    }
};

