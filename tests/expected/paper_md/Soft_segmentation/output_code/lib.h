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

struct soft {
    using DT = double;
    std::function<double(int, Eigen::Matrix<double, 3, 1>)> D;
    std::vector<Eigen::Matrix<double, 3, 1>> boldsymbolu;
    double σ;
    std::vector<double> α;
    long boldsymbolu_n;
    long α_n;
    double F_S;

    soft(const std::function<double(int, Eigen::Matrix<double, 3, 1>)> & D, const std::vector<Eigen::Matrix<double, 3, 1>> & boldsymbolu, const double & σ, const std::vector<double> & α)
    {
        this->D = D;
        this->boldsymbolu = boldsymbolu;
        this->σ = σ;
        this->α = α;
        this->boldsymbolu_n = boldsymbolu.size();
        this->α_n = α.size();
        // `$F_S$` = sum_i α_i D_i(`$\boldsymbol{u}$`_i) + σ((sum_i α_i)/(sum_i α_i^2) - 1)
        this->F_S = [&](){ double sum = 0.0; for(int i = 1; i <= this->α_n; i++){ sum += this->α.at(i-1) * this->D(i, this->boldsymbolu.at(i-1)); } return sum; }() + this->σ * ([&](){ double sum = 0.0; for(int i = 1; i <= this->α_n; i++){ sum += this->α.at(i-1); } return sum; }() / [&](){ double sum = 0.0; for(int i = 1; i <= this->α_n; i++){ sum += pow(this->α.at(i-1), 2); } return sum; }() - 1);
    }
};

