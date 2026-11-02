#pragma once
#include <Eigen/Core>
#include <Eigen/QR>
#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <iostream>
#include <set>
#include <algorithm>
#include "type_helper.h"

struct Regularized {
    using DT = double;
    using MatrixD = Eigen::MatrixXd;
    using VectorD = Eigen::VectorXd;

    double a;
    double b;
    double r;
    Eigen::Matrix<double, 3, 1> boldsymbolf;
    double r_ε;
    double ε;
    Eigen::Matrix<double, 3, 3> boldsymbolF;
    double s;
    template<typename REAL>
    Eigen::Matrix<REAL, 3, 1> boldsymbolu(
        const Eigen::Matrix<REAL, 3, 1> & boldsymbolr)
    {
        return ((this->a - this->b) / REAL(this->r) * MatrixD::Identity(3, 3) + this->b / REAL(pow(this->r, 3)) * boldsymbolr * boldsymbolr.transpose()) * this->boldsymbolf;    
    }
    template<typename REAL>
    REAL rho_ε(
        const Eigen::Matrix<REAL, 3, 1> & boldsymbolr)
    {
        return (15 * pow(this->r_ε, 4) / REAL((8 * M_PI)) + 1 / REAL(pow(this->r_ε, 7)));    
    }
    template<typename REAL>
    Eigen::Matrix<REAL, 3, 1> boldsymbolu_ε(
        const Eigen::Matrix<REAL, 3, 1> & boldsymbolr)
    {
        return ((this->a - this->b) / REAL(this->r_ε) * MatrixD::Identity(3, 3) + this->b / REAL(pow(this->r_ε, 3)) * boldsymbolr * boldsymbolr.transpose() + this->a * pow(this->ε, 2) / REAL((2 * pow(this->r_ε, 3))) * MatrixD::Identity(3, 3)) * this->boldsymbolf;    
    }
    template<typename REAL>
    Eigen::Matrix<REAL, 3, 1> tildeboldsymbolu_ε(
        const Eigen::Matrix<REAL, 3, 1> & boldsymbolr)
    {
        return -this->a * (1 / REAL(pow(this->r_ε, 3)) + 3 * pow(this->ε, 2) / REAL((2 * pow(this->r_ε, 5)))) * this->boldsymbolF * boldsymbolr + this->b * (1 / REAL(pow(this->r_ε, 3)) * (this->boldsymbolF + this->boldsymbolF.transpose() + (this->boldsymbolF).trace() * MatrixD::Identity(3, 3)) - 3 / REAL(pow(this->r_ε, 5)) * ((boldsymbolr.transpose() * this->boldsymbolF * boldsymbolr).value()) * MatrixD::Identity(3, 3)) * boldsymbolr;    
    }
    template<typename REAL>
    Eigen::Matrix<REAL, 3, 1> boldsymbolt_ε(
        const Eigen::Matrix<REAL, 3, 1> & boldsymbolr)
    {
        return -this->a * (1 / REAL(pow(this->r_ε, 3)) + 3 * pow(this->ε, 2) / REAL((2 * pow(this->r_ε, 5)))) * this->boldsymbolF * boldsymbolr;    
    }
    template<typename REAL>
    Eigen::Matrix<REAL, 3, 1> boldsymbols_ε(
        const Eigen::Matrix<REAL, 3, 1> & boldsymbolr)
    {
        return (2 * this->b - this->a) * (1 / REAL(pow(this->r_ε, 3)) + 3 * pow(this->ε, 2) / REAL((2 * pow(this->r_ε, 5)))) * (this->s * boldsymbolr);    
    }
    template<typename REAL>
    Eigen::Matrix<REAL, 3, 1> boldsymbolp_ε(
        const Eigen::Matrix<REAL, 3, 1> & boldsymbolr)
    {
        return (2 * this->b - this->a) / REAL(pow(this->r_ε, 3)) * this->boldsymbolF * boldsymbolr - 3 / REAL((2 * pow(this->r_ε, 5))) * (2 * this->b * ((boldsymbolr.transpose() * this->boldsymbolF * boldsymbolr).value()) * MatrixD::Identity(3, 3) + this->a * pow(this->ε, 2) * this->boldsymbolF) * boldsymbolr;    
    }
    Regularized(
        const double & r,
        const Eigen::Matrix<double, 3, 1> & boldsymbolf,
        const double & s,
        const Eigen::Matrix<double, 3, 3> & boldsymbolF,
        const double & r_ε,
        const double & a,
        const double & b,
        const double & ε)
    {
    
        this->a = a;
        this->b = b;
        this->r = r;
        this->boldsymbolf = boldsymbolf;
        this->r_ε = r_ε;
        this->ε = ε;
        this->boldsymbolF = boldsymbolF;
        this->s = s;
    
    }
};

