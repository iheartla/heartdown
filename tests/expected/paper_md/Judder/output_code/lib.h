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

struct judder {
    using DT = double;
    double F;
    double L;
    double S;
    std::function<double(double, double, double)> P;
    double a;
    double b;
    double M;
    double L_a;
    double L_b;
    double F_a;
    double F_b;
    double J;

    judder(const double & F, const double & L, const double & S, const std::function<double(double, double, double)> & P, const double & a, const double & b, const double & M, const double & L_a, const double & L_b)
    {
        this->F = F;
        this->L = L;
        this->S = S;
        this->P = P;
        this->a = a;
        this->b = b;
        this->M = M;
        this->L_a = L_a;
        this->L_b = L_b;
        // `$F_a$` = M⋅ CFF(`$L_a$`)
        this->F_a = this->M * this->CFF(this->L_a);
        // `$F_b$` = M⋅ CFF(`$L_b$`)
        this->F_b = this->M * this->CFF(this->L_b);
        // J = P(α(F), β(L), S)
        this->J = this->P(this->α(this->F), this->β(this->L), this->S);
    }

    double CFF(const double & L)
    {
        return this->a * log(L) + this->b;
    }

    double α(const double & F)
    {
        return 1 / F;
    }

    double β(const double & L)
    {
        return log10(L);
    }
};

struct error {
    using DT = double;
    Eigen::VectorXd O;
    Eigen::VectorXd M;
    long N;
    double E;

    error(const Eigen::VectorXd & O, const Eigen::VectorXd & M)
    {
        this->O = O;
        this->M = M;
        this->N = O.rows();
        assert( O.rows() == this->N );
        assert( M.rows() == this->N );
        // E = sum_i |log(O_i) - log(M_i)|/log(O_i)
        this->E = [&](){ double sum = 0.0; for(int i = 1; i <= this->N; i++){ sum += fabs(log(this->O(i-1)) - log(this->M(i-1))) / log(this->O(i-1)); } return sum; }();
    }
};

struct third {
    using DT = double;
    double a;
    double b;
    double F_b;
    double F_a;
    double L_a;
    double L_b;

    third(const double & a, const double & b, const double & F_b, const double & F_a, const double & L_a)
    {
        this->a = a;
        this->b = b;
        this->F_b = F_b;
        this->F_a = F_a;
        this->L_a = L_a;
        // `$L_b$` = 10^((a `$F_b$`log((`$L_a$`))+b(`$F_b$`-`$F_a$`))/(a`$F_a$`))
        this->L_b = pow(10, (((this->a * this->F_b) * log(this->L_a) + this->b * (this->F_b - this->F_a)) / (this->a * this->F_a)));
    }
};

