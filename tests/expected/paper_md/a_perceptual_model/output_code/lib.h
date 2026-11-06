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

struct perceptual {
    using DT = double;
    std::function<double(double)> CSF;
    std::vector<double> ω;
    double m_combining_tilde_t_comma_b;
    double β_b;
    double σ_P_circumflex_accent_A;
    double σ_P_circumflex_accent_B;
    double σ_O_circumflex_accent_A;
    double σ_O_circumflex_accent_B;
    long ω_n;
    double increment_Q_P;
    double increment_Q_O;

    perceptual(const std::function<double(double)> & CSF, const std::vector<double> & ω, const double & m_combining_tilde_t_comma_b, const double & β_b, const double & σ_P_circumflex_accent_A, const double & σ_P_circumflex_accent_B, const double & σ_O_circumflex_accent_A, const double & σ_O_circumflex_accent_B)
    {
        this->CSF = CSF;
        this->ω = ω;
        this->m_combining_tilde_t_comma_b = m_combining_tilde_t_comma_b;
        this->β_b = β_b;
        this->σ_P_circumflex_accent_A = σ_P_circumflex_accent_A;
        this->σ_P_circumflex_accent_B = σ_P_circumflex_accent_B;
        this->σ_O_circumflex_accent_A = σ_O_circumflex_accent_A;
        this->σ_O_circumflex_accent_B = σ_O_circumflex_accent_B;
        this->ω_n = ω.size();
        // `$∆Q_P$` = `$E_b$`(`$σ_P^A$`) - `$E_b$`(`$σ_P^B$`)
        this->increment_Q_P = this->E_b(this->σ_P_circumflex_accent_A) - this->E_b(this->σ_P_circumflex_accent_B);
        // `$∆Q_O$` = `$E_b$`(`$σ_O^A$`) - `$E_b$`(`$σ_O^B$`)
        this->increment_Q_O = this->E_b(this->σ_O_circumflex_accent_A) - this->E_b(this->σ_O_circumflex_accent_B);
    }

    double m(const double & ω, const double & σ)
    {
        return exp(-(((2 * pow(M_PI, 2)) * pow(ω, 2)) * pow(σ, 2)));
    }

    double m̃(const double & ω, const double & σ)
    {
        return this->CSF(ω) * this->m(ω, σ);
    }

    double E_b(const double & σ)
    {
        return [&](){ double sum = 0.0; for(int i = 1; i <= this->ω_n; i++){ sum += pow((this->m̃(this->ω.at(i-1), σ) / this->m_combining_tilde_t_comma_b), this->β_b); } return sum; }();
    }
};

