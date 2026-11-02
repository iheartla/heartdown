#pragma once
#include <Eigen/Core>
#include <Eigen/QR>
#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <iostream>
#include <set>
#include <algorithm>
#include "type_helper.h"

struct perceptual {
    using DT = double;
    using MatrixD = Eigen::MatrixXd;
    using VectorD = Eigen::VectorXd;
    DT increment_Q_P;
    DT increment_Q_O;
    std::vector<double> ω;
    std::function<double(double)> CSF;
    double m_combining_tilde_t_comma_b;
    double β_b;
    template<typename REAL>
    REAL m(
        const REAL & ω,
        const REAL & σ)
    {
        return exp(-2 * pow(M_PI, 2) * pow(ω, 2) * pow(σ, 2));    
    }
    template<typename REAL>
    REAL m̃(
        const REAL & ω,
        const REAL & σ)
    {
        return this->CSF(ω) * m(ω, σ);    
    }
    template<typename REAL>
    REAL E_b(
        const REAL & σ)
    {
        REAL sum_0 = 0;
        for(int i=1; i<=ω.size(); i++){
            sum_0 += pow(((m̃(this->ω.at(i-1), σ)) / REAL(this->m_combining_tilde_t_comma_b)), this->β_b);
        }
        return sum_0;    
    }
    perceptual(
        const std::function<double(double)> & CSF,
        const std::vector<double> & ω,
        const double & m_combining_tilde_t_comma_b,
        const double & β_b,
        const double & σ_P_circumflex_accent_A,
        const double & σ_P_circumflex_accent_B,
        const double & σ_O_circumflex_accent_A,
        const double & σ_O_circumflex_accent_B)
    {
        const long dim_0 = ω.size();
        this->ω = ω;
        this->CSF = CSF;
        this->m_combining_tilde_t_comma_b = m_combining_tilde_t_comma_b;
        this->β_b = β_b;
        // `$∆Q_P$` = `$E_b$`(`$σ_P^A$`) - `$E_b$`(`$σ_P^B$`)
        increment_Q_P = E_b(σ_P_circumflex_accent_A) - E_b(σ_P_circumflex_accent_B);
        // `$∆Q_O$` = `$E_b$`(`$σ_O^A$`) - `$E_b$`(`$σ_O^B$`)
        increment_Q_O = E_b(σ_O_circumflex_accent_A) - E_b(σ_O_circumflex_accent_B);
    }
};

