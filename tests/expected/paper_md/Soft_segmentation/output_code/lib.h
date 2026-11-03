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

struct soft {
    using DT = double;
    std::function<double(int, Eigen::Matrix<double, 3, 1>)> D;
    std::vector<Eigen::Matrix<double, 3, 1>> boldsymbolu;
    double σ;
    std::vector<double> α;
    long boldsymbolu_n;
    long α_n;
    double F_S;

    soft() {}
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

