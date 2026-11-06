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

struct Regularized {
    using DT = double;
    double r;
    Eigen::Matrix<double, 3, 1> boldsymbolf;
    double s;
    Eigen::Matrix<double, 3, 3> boldsymbolF;
    double r_ε;
    double a;
    double b;
    double ε;

    Regularized(const double & r, const Eigen::Matrix<double, 3, 1> & boldsymbolf, const double & s, const Eigen::Matrix<double, 3, 3> & boldsymbolF, const double & r_ε, const double & a, const double & b, const double & ε)
    {
        this->r = r;
        this->boldsymbolf = boldsymbolf;
        this->s = s;
        this->boldsymbolF = boldsymbolF;
        this->r_ε = r_ε;
        this->a = a;
        this->b = b;
        this->ε = ε;
    }

    Eigen::Matrix<double, 3, 1> boldsymbolu(const Eigen::Matrix<double, 3, 1> & boldsymbolr)
    {
        return (((this->a - this->b) / this->r) * Eigen::Matrix<double, 3, 3>::Identity(3, 3) + ((this->b / pow(this->r, 3)) * boldsymbolr) * boldsymbolr.transpose()) * this->boldsymbolf;
    }

    double rho_ε(const Eigen::Matrix<double, 3, 1> & boldsymbolr)
    {
        return ((15 * pow(this->r_ε, 4)) / (8 * M_PI) + 1 / pow(this->r_ε, 7));
    }

    Eigen::Matrix<double, 3, 1> boldsymbolu_ε(const Eigen::Matrix<double, 3, 1> & boldsymbolr)
    {
        return (((this->a - this->b) / this->r_ε) * Eigen::Matrix<double, 3, 3>::Identity(3, 3) + ((this->b / pow(this->r_ε, 3)) * boldsymbolr) * boldsymbolr.transpose() + ((this->a * pow(this->ε, 2)) / (2 * pow(this->r_ε, 3))) * Eigen::Matrix<double, 3, 3>::Identity(3, 3)) * this->boldsymbolf;
    }

    Eigen::Matrix<double, 3, 1> tildeboldsymbolu_ε(const Eigen::Matrix<double, 3, 1> & boldsymbolr)
    {
        return -(((this->a * (1 / pow(this->r_ε, 3) + (3 * pow(this->ε, 2)) / (2 * pow(this->r_ε, 5)))) * this->boldsymbolF) * boldsymbolr) + (this->b * ((1 / pow(this->r_ε, 3)) * (this->boldsymbolF + this->boldsymbolF.transpose() + (this->boldsymbolF).trace() * Eigen::Matrix<double, 3, 3>::Identity(3, 3)) - ((3 / pow(this->r_ε, 5)) * (((boldsymbolr.transpose() * this->boldsymbolF) * boldsymbolr).value())) * Eigen::Matrix<double, 3, 3>::Identity(3, 3))) * boldsymbolr;
    }

    Eigen::Matrix<double, 3, 1> boldsymbolt_ε(const Eigen::Matrix<double, 3, 1> & boldsymbolr)
    {
        return -(((this->a * (1 / pow(this->r_ε, 3) + (3 * pow(this->ε, 2)) / (2 * pow(this->r_ε, 5)))) * this->boldsymbolF) * boldsymbolr);
    }

    Eigen::Matrix<double, 3, 1> boldsymbols_ε(const Eigen::Matrix<double, 3, 1> & boldsymbolr)
    {
        return ((2 * this->b - this->a) * (1 / pow(this->r_ε, 3) + (3 * pow(this->ε, 2)) / (2 * pow(this->r_ε, 5)))) * (this->s * boldsymbolr);
    }

    Eigen::Matrix<double, 3, 1> boldsymbolp_ε(const Eigen::Matrix<double, 3, 1> & boldsymbolr)
    {
        return (((2 * this->b - this->a) / pow(this->r_ε, 3)) * this->boldsymbolF) * boldsymbolr - ((3 / (2 * pow(this->r_ε, 5))) * (((2 * this->b) * (((boldsymbolr.transpose() * this->boldsymbolF) * boldsymbolr).value())) * Eigen::Matrix<double, 3, 3>::Identity(3, 3) + (this->a * pow(this->ε, 2)) * this->boldsymbolF)) * boldsymbolr;
    }
};

