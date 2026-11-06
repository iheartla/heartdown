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

struct computation {
    using DT = double;
    double λ_s;
    double E_a;
    std::function<double(double, double)> D;
    double S_1;
    double S_2;
    double U_1;
    double U_2;
    std::vector<Eigen::Matrix<double, 3, 1>> R;
    long R_n;
    double E_c;
    double E_s;
    double E_f;
    double E;

    computation(const double & λ_s, const double & E_a, const std::function<double(double, double)> & D, const double & S_1, const double & S_2, const double & U_1, const double & U_2, const std::vector<Eigen::Matrix<double, 3, 1>> & R)
    {
        this->λ_s = λ_s;
        this->E_a = E_a;
        this->D = D;
        this->S_1 = S_1;
        this->S_2 = S_2;
        this->U_1 = U_1;
        this->U_2 = U_2;
        this->R = R;
        this->R_n = R.size();
        // `$E_c$` = D(`$S_1$`, `$U_1$`) + D(`$S_2$`, `$U_2$`)
        this->E_c = this->D(this->S_1, this->U_1) + this->D(this->S_2, this->U_2);
        // `$E_s$` = `$E_c$` + `$E_a$`
        this->E_s = this->E_c + this->E_a;
        // `$E_f$` = sum_i ||R_i||
        this->E_f = [&](){ double sum = 0.0; for(int i = 1; i <= this->R_n; i++){ sum += (this->R.at(i-1)).norm(); } return sum; }();
        // E = `$λ_s$``$E_s$` + (1 - `$λ_s$`)`$E_f$`
        this->E = this->λ_s * this->E_s + (1 - this->λ_s) * this->E_f;
    }
};

