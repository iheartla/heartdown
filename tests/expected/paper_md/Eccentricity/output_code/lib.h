#pragma once
#include <Eigen/Core>
#include <Eigen/QR>
#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <iostream>
#include <set>
#include <algorithm>
#include "type_helper.h"

struct eccentricity {
    using DT = double;
    using MatrixD = Eigen::MatrixXd;
    using VectorD = Eigen::VectorXd;
    int 𝑙_0;
    Eigen::Matrix<DT, 3, 1> q;
    std::function<double(double, double)> m;
    double 𝑓_𝑠0;
    Eigen::Matrix<double, 10, 1> 𝑝;
    double a;
    template<typename REAL>
    REAL 𝑔(
        const Eigen::Matrix<REAL, 2, 1> & x,
        const Eigen::Matrix<REAL, 2, 1> & x_0,
        const REAL & 𝜃,
        const REAL & 𝜎,
        const REAL & 𝑓_𝑠)
    {
        Eigen::Matrix<REAL, 2, 1> 𝑔_1;
        𝑔_1 << cos(𝜃), sin(𝜃);
        return exp(-pow((x - x_0).template lpNorm<2>(), 2) / REAL((2 * pow(𝜎, 2)))) * cos((2 * M_PI * 𝑓_𝑠 * x).dot(𝑔_1));    
    }
    template<typename REAL>
    REAL 𝜏(
        const REAL & 𝑓_𝑠)
    {
        return this->m(log10(𝑓_𝑠) - log10(this->𝑓_𝑠0), REAL(0));    
    }
    template<typename REAL>
    REAL 𝜁(
        const REAL & 𝑓_𝑠)
    {
        return exp(this->𝑝[10-1] * 𝜏(𝑓_𝑠)) - 1;    
    }
    template<typename REAL>
    REAL Ψ(
        const REAL & 𝑒,
        const REAL & 𝑓_𝑠)
    {
        return this->m(REAL(0), this->𝑝[1-1] * pow(𝜏(𝑓_𝑠), 2) + this->𝑝[2-1] * 𝜏(𝑓_𝑠) + this->𝑝[3-1] + (this->𝑝[4-1] * pow(𝜏(𝑓_𝑠), 2) + this->𝑝[5-1] * 𝜏(𝑓_𝑠) + this->𝑝[6-1]) * 𝜁(𝑓_𝑠) * 𝑒 + (this->𝑝[7-1] * pow(𝜏(𝑓_𝑠), 2) + this->𝑝[8-1] * 𝜏(𝑓_𝑠) + this->𝑝[9-1]) * 𝜁(𝑓_𝑠) * pow(𝑒, 2));    
    }
    template<typename REAL>
    REAL 𝐴(
        const REAL & 𝑒)
    {
        return log(64) * 2.3 / REAL((0.106 * (𝑒 + 2.3)));    
    }
    template<typename REAL>
    REAL 𝑑(
        const REAL & 𝐿)
    {
        return 7.75 - 5.75 * (pow((𝐿 * this->a / REAL(846)), 0.41) / REAL((pow((𝐿 * this->a / REAL(846)), 0.41) + 2)));    
    }
    template<typename REAL>
    REAL 𝑙(
        const REAL & 𝐿)
    {
        return M_PI * pow(𝑑(𝐿), 2) / REAL(4) * 𝐿;    
    }
    template<typename REAL>
    REAL 𝑠(
        const REAL & 𝑒,
        const REAL & 𝑓_𝑠)
    {
        return 𝜁(𝑓_𝑠) * (this->q[1-1] * pow(𝑒, 2) + this->q[2-1] * 𝑒) + this->q[3-1];    
    }
    template<typename REAL>
    REAL hatΨ(
        const REAL & 𝑒,
        const REAL & 𝑓_𝑠,
        const REAL & 𝐿)
    {
        return (𝑠(𝑒, 𝑓_𝑠) * (log10(𝑙(𝐿) / REAL(this->𝑙_0))) + 1) * Ψ(𝑒, 𝑓_𝑠);    
    }
    eccentricity(
        const std::function<double(double, double)> & m,
        const Eigen::Matrix<double, 10, 1> & 𝑝,
        const double & 𝑓_𝑠0,
        const double & a)
    {
    
        this->m = m;
        this->𝑓_𝑠0 = 𝑓_𝑠0;
        this->𝑝 = 𝑝;
        this->a = a;
        // `$𝑙_0$` = 1488
        𝑙_0 = 1488;
        // q = (5.71 ⋅ 10^(-6), -1.78 ⋅ 10^(-4), 0.204)
        Eigen::Matrix<DT, 3, 1> q_0;
        q_0 << 5.71 * pow(10, (-6)), -1.78 * pow(10, (-4)), 0.204;
        q = q_0;
    }
};

