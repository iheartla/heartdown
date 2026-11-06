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

struct ICP {
    using DT = double;
    Eigen::Matrix<double, 3, 3> R;
    Eigen::Matrix<double, 3, 1> a;
    double θ;
    std::vector<Eigen::Matrix<double, 3, 1>> p;
    std::vector<Eigen::Matrix<double, 3, 1>> q;
    std::vector<Eigen::Matrix<double, 3, 1>> n_q;
    std::vector<Eigen::Matrix<double, 3, 1>> n_p;
    Eigen::Matrix<double, 3, 1> t;
    Eigen::Matrix<double, 3, 1> barq;
    Eigen::Matrix<double, 3, 1> barp;
    std::function<Eigen::Matrix<double, 4, 4>(Eigen::Matrix<double, 3, 1>)> trans;
    std::function<Eigen::Matrix<double, 4, 4>(double, Eigen::Matrix<double, 3, 1>)> rot;
    long p_n;
    long q_n;
    long n_q_n;
    long n_p_n;
    Eigen::Matrix<double, 3, 1> ã;
    std::vector<Eigen::Matrix<double, 3, 1>> n;
    Eigen::Matrix<double, 3, 1> t̃;
    std::vector<Eigen::Matrix<double, 3, 1>> p̃;
    std::vector<Eigen::Matrix<double, 3, 1>> q̃;
    double varepsilon_point;
    double varepsilon_plane;
    double varepsilon_symmRN;
    double varepsilon_symm;
    Eigen::Matrix<double, 4, 4> S;
    double varepsilon_twoplane;

    ICP(const Eigen::Matrix<double, 3, 3> & R, const Eigen::Matrix<double, 3, 1> & a, const double & θ, const std::vector<Eigen::Matrix<double, 3, 1>> & p, const std::vector<Eigen::Matrix<double, 3, 1>> & q, const std::vector<Eigen::Matrix<double, 3, 1>> & n_q, const std::vector<Eigen::Matrix<double, 3, 1>> & n_p, const Eigen::Matrix<double, 3, 1> & t, const Eigen::Matrix<double, 3, 1> & barq, const Eigen::Matrix<double, 3, 1> & barp, const std::function<Eigen::Matrix<double, 4, 4>(Eigen::Matrix<double, 3, 1>)> & trans, const std::function<Eigen::Matrix<double, 4, 4>(double, Eigen::Matrix<double, 3, 1>)> & rot)
    {
        this->R = R;
        this->a = a;
        this->θ = θ;
        this->p = p;
        this->q = q;
        this->n_q = n_q;
        this->n_p = n_p;
        this->t = t;
        this->barq = barq;
        this->barp = barp;
        this->trans = trans;
        this->rot = rot;
        this->p_n = p.size();
        this->q_n = q.size();
        this->n_q_n = n_q.size();
        this->n_p_n = n_p.size();
        // ã = a tan(θ)
        this->ã = this->a * tan(this->θ);
        // n_i = `$n_q$`_i + `$n_p$`_i
        this->n.clear();
        for(int i = 1; i <= this->n_q_n; i++){
            this->n.push_back(this->n_q.at(i-1) + this->n_p.at(i-1));
        }
        // t̃ = t/cos(θ)
        this->t̃ = this->t / cos(this->θ);
        // p̃_i = p_i - `$\bar{p}$`
        this->p̃.clear();
        for(int i = 1; i <= this->p_n; i++){
            this->p̃.push_back(this->p.at(i-1) - this->barp);
        }
        // q̃_i = q_i - `$\bar{q}$`
        this->q̃.clear();
        for(int i = 1; i <= this->q_n; i++){
            this->q̃.push_back(this->q.at(i-1) - this->barq);
        }
        // `$\varepsilon_{point}$` = ∑_i ||R p_i + t - q_i||
        this->varepsilon_point = [&](){ double sum = 0.0; for(int i = 1; i <= this->p_n; i++){ sum += (this->R * this->p.at(i-1) + this->t - this->q.at(i-1)).norm(); } return sum; }();
        // `$\varepsilon_{plane}$` = ∑_i ((R p_i + t - q_i) ⋅ `$n_q$`_i)^2
        this->varepsilon_plane = [&](){ double sum = 0.0; for(int i = 1; i <= this->p_n; i++){ sum += pow(((((this->R * this->p.at(i-1) + this->t - this->q.at(i-1))).eval()).dot((this->n_q.at(i-1)).eval())), 2); } return sum; }();
        // `$\varepsilon_{symm-RN}$` = ∑_i ((R p_i + R⁻¹ q_i + t) ⋅ (R`$n_p$`_i + R⁻¹`$n_q$`_i))^2
        this->varepsilon_symmRN = [&](){ double sum = 0.0; for(int i = 1; i <= this->p_n; i++){ sum += pow(((((this->R * this->p.at(i-1) + ((this->R).eval()).colPivHouseholderQr().solve((this->q.at(i-1)).eval()) + this->t)).eval()).dot(((this->R * this->n_p.at(i-1) + ((this->R).eval()).colPivHouseholderQr().solve((this->n_q.at(i-1)).eval()))).eval())), 2); } return sum; }();
        // `$\varepsilon_{symm}$` = ∑_i cos²(θ)((p_i - q_i)⋅n_i +((p_i+q_i)×n_i)⋅ã+n_i⋅t̃)²
        this->varepsilon_symm = [&](){ double sum = 0.0; for(int i = 1; i <= this->p_n; i++){ sum += pow(cos(this->θ), 2) * pow(((((this->p.at(i-1) - this->q.at(i-1))).eval()).dot((this->n.at(i-1)).eval()) + ((((((this->p.at(i-1) + this->q.at(i-1))).eval()).cross((this->n.at(i-1)).eval()))).eval()).dot((this->ã).eval()) + ((this->n.at(i-1)).eval()).dot((this->t̃).eval())), 2); } return sum; }();
        // S = trans(`$\bar{q}$`) ⋅ rot(θ, ã/||ã||) ⋅trans(t̃ cos(θ)) ⋅rot(θ, ã/||ã||)⋅ trans(-`$\bar{p}$`)
        this->S = (((this->trans(this->barq) * this->rot(this->θ, this->ã / (this->ã).norm())) * this->trans(this->t̃ * cos(this->θ))) * this->rot(this->θ, this->ã / (this->ã).norm())) * this->trans(-this->barp);
        // `$\varepsilon_{two-plane}$` = ∑_i(((R p_i + R⁻¹ q_i + t) ⋅ (R `$n_p$`_i))^2 + ((R p_i + R⁻¹ q_i + t) ⋅ (R⁻¹`$n_q$`_i))^2)
        this->varepsilon_twoplane = [&](){ double sum = 0.0; for(int i = 1; i <= this->p_n; i++){ sum += (pow(((((this->R * this->p.at(i-1) + ((this->R).eval()).colPivHouseholderQr().solve((this->q.at(i-1)).eval()) + this->t)).eval()).dot(((this->R * this->n_p.at(i-1))).eval())), 2) + pow(((((this->R * this->p.at(i-1) + ((this->R).eval()).colPivHouseholderQr().solve((this->q.at(i-1)).eval()) + this->t)).eval()).dot(((((this->R).eval()).colPivHouseholderQr().solve((this->n_q.at(i-1)).eval()))).eval())), 2)); } return sum; }();
    }
};

