#pragma once
#include <Eigen/Core>
#include <Eigen/QR>
#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <iostream>
#include <set>
#include <algorithm>
#include "type_helper.h"

struct ICP {
    using DT = double;
    using MatrixD = Eigen::MatrixXd;
    using VectorD = Eigen::VectorXd;
    Eigen::Matrix<DT, 3, 1> ã;
    std::vector<Eigen::Matrix<DT, 3, 1>> n;
    Eigen::Matrix<DT, 3, 1> t̃;
    std::vector<Eigen::Matrix<DT, 3, 1>> p̃;
    std::vector<Eigen::Matrix<DT, 3, 1>> q̃;
    DT varepsilon_point;
    DT varepsilon_plane;
    DT varepsilon_symmRN;
    DT varepsilon_symm;
    Eigen::Matrix<DT, 4, 4> S;
    DT varepsilon_twoplane;

    ICP(
        const Eigen::Matrix<double, 3, 3> & R,
        const Eigen::Matrix<double, 3, 1> & a,
        const double & θ,
        const std::vector<Eigen::Matrix<double, 3, 1>> & p,
        const std::vector<Eigen::Matrix<double, 3, 1>> & q,
        const std::vector<Eigen::Matrix<double, 3, 1>> & n_q,
        const std::vector<Eigen::Matrix<double, 3, 1>> & n_p,
        const Eigen::Matrix<double, 3, 1> & t,
        const Eigen::Matrix<double, 3, 1> & barq,
        const Eigen::Matrix<double, 3, 1> & barp,
        const std::function<Eigen::Matrix<double, 4, 4>(Eigen::Matrix<double, 3, 1>)> & trans,
        const std::function<Eigen::Matrix<double, 4, 4>(double, Eigen::Matrix<double, 3, 1>)> & rot)
    {
        const long dim_0 = p.size();
        assert( q.size() == dim_0 );
        assert( n_q.size() == dim_0 );
        assert( n_p.size() == dim_0 );
        // ã = a tan(θ)
        ã = a * tan(θ);
        // n_i = `$n_q$`_i + `$n_p$`_i
        n.resize(dim_0);
        for( int i=1; i<=dim_0; i++){
            n.at(i-1) = n_q.at(i-1) + n_p.at(i-1);
        }
        // t̃ = t/cos(θ)
        t̃ = t / DT(cos(θ));
        // p̃_i = p_i - `$\bar{p}$`
        p̃.resize(dim_0);
        for( int i=1; i<=dim_0; i++){
            p̃.at(i-1) = p.at(i-1) - barp;
        }
        // q̃_i = q_i - `$\bar{q}$`
        q̃.resize(dim_0);
        for( int i=1; i<=dim_0; i++){
            q̃.at(i-1) = q.at(i-1) - barq;
        }
        // `$\varepsilon_{point}$` = ∑_i ||R p_i + t - q_i||
        DT sum_0 = 0;
        for(int i=1; i<=p.size(); i++){
            sum_0 += (R * p.at(i-1) + t - q.at(i-1)).template lpNorm<2>();
        }
        varepsilon_point = sum_0;
        // `$\varepsilon_{plane}$` = ∑_i ((R p_i + t - q_i) ⋅ `$n_q$`_i)^2
        DT sum_1 = 0;
        for(int i=1; i<=n_q.size(); i++){
            sum_1 += pow((((R * p.at(i-1) + t - q.at(i-1))).dot(n_q.at(i-1))), 2);
        }
        varepsilon_plane = sum_1;
        // `$\varepsilon_{symm-RN}$` = ∑_i ((R p_i + R⁻¹ q_i + t) ⋅ (R`$n_p$`_i + R⁻¹`$n_q$`_i))^2
        DT sum_2 = 0;
        for(int i=1; i<=n_p.size(); i++){
            sum_2 += pow((((R * p.at(i-1) + (to_double(R)).colPivHouseholderQr().solve(to_double(q.at(i-1))) + t)).dot((R * n_p.at(i-1) + (to_double(R)).colPivHouseholderQr().solve(to_double(n_q.at(i-1)))))), 2);
        }
        varepsilon_symmRN = sum_2;
        // `$\varepsilon_{symm}$` = ∑_i cos²(θ)((p_i - q_i)⋅n_i +((p_i+q_i)×n_i)⋅ã+n_i⋅t̃)²
        DT sum_3 = 0;
        for(int i=1; i<=n.size(); i++){
            sum_3 += pow(cos(θ), 2) * pow((((p.at(i-1) - q.at(i-1))).dot(this->n.at(i-1)) + ((((p.at(i-1) + q.at(i-1))).cross(this->n.at(i-1)))).dot(this->ã) + (this->n.at(i-1)).dot(this->t̃)), 2);
        }
        varepsilon_symm = sum_3;
        // S = trans(`$\bar{q}$`) ⋅ rot(θ, ã/||ã||) ⋅trans(t̃ cos(θ)) ⋅rot(θ, ã/||ã||)⋅ trans(-`$\bar{p}$`)
        S = trans((barq).eval()) * rot(θ, (ã / DT((ã).template lpNorm<2>())).eval()) * trans((t̃ * cos(θ)).eval()) * rot(θ, (ã / DT((ã).template lpNorm<2>())).eval()) * trans((-barp).eval());
        // `$\varepsilon_{two-plane}$` = ∑_i(((R p_i + R⁻¹ q_i + t) ⋅ (R `$n_p$`_i))^2 + ((R p_i + R⁻¹ q_i + t) ⋅ (R⁻¹`$n_q$`_i))^2)
        DT sum_4 = 0;
        for(int i=1; i<=n_p.size(); i++){
            sum_4 += (pow((((R * p.at(i-1) + (to_double(R)).colPivHouseholderQr().solve(to_double(q.at(i-1))) + t)).dot((R * n_p.at(i-1)))), 2) + pow((((R * p.at(i-1) + (to_double(R)).colPivHouseholderQr().solve(to_double(q.at(i-1))) + t)).dot(((to_double(R)).colPivHouseholderQr().solve(to_double(n_q.at(i-1)))))), 2));
        }
        varepsilon_twoplane = sum_4;
    }
};

