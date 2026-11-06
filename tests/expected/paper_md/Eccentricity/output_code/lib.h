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

struct eccentricity {
    using DT = double;
    std::function<double(double, double)> m;
    Eigen::Matrix<double, 10, 1> 𝑝;
    double 𝑓_𝑠0;
    double a;
    int 𝑙_0;
    Eigen::Matrix<double, 3, 1> q;

    eccentricity(const std::function<double(double, double)> & m, const Eigen::Matrix<double, 10, 1> & 𝑝, const double & 𝑓_𝑠0, const double & a)
    {
        this->m = m;
        this->𝑝 = 𝑝;
        this->𝑓_𝑠0 = 𝑓_𝑠0;
        this->a = a;
        // `$𝑙_0$` = 1488
        this->𝑙_0 = 1488;
        // q = (5.71 ⋅ 10^(-6), -1.78 ⋅ 10^(-4), 0.204)
        this->q = [&](){ Eigen::Matrix<double, 3, 1> v(3); v << 5.71 * pow(10, (-6)), -(1.78 * pow(10, (-4))), 0.204; return v; }();
    }

    double 𝑔(const Eigen::Matrix<double, 2, 1> & x, const Eigen::Matrix<double, 2, 1> & x_0, const double & 𝜃, const double & 𝜎, const double & 𝑓_𝑠)
    {
        return exp(-(pow((x - x_0).norm(), 2) / (2 * pow(𝜎, 2)))) * cos(((((2 * M_PI) * 𝑓_𝑠) * x).eval()).dot(([&](){ Eigen::Matrix<double, 2, 1> v(2); v << cos(𝜃), sin(𝜃); return v; }()).eval()));
    }

    double 𝜏(const double & 𝑓_𝑠)
    {
        return this->m(log10(𝑓_𝑠) - log10(this->𝑓_𝑠0), 0);
    }

    double 𝜁(const double & 𝑓_𝑠)
    {
        return exp(this->𝑝(9) * this->𝜏(𝑓_𝑠)) - 1;
    }

    double Ψ(const double & 𝑒, const double & 𝑓_𝑠)
    {
        return this->m(0, this->𝑝(0) * pow(this->𝜏(𝑓_𝑠), 2) + this->𝑝(1) * this->𝜏(𝑓_𝑠) + this->𝑝(2) + ((this->𝑝(3) * pow(this->𝜏(𝑓_𝑠), 2) + this->𝑝(4) * this->𝜏(𝑓_𝑠) + this->𝑝(5)) * this->𝜁(𝑓_𝑠)) * 𝑒 + ((this->𝑝(6) * pow(this->𝜏(𝑓_𝑠), 2) + this->𝑝(7) * this->𝜏(𝑓_𝑠) + this->𝑝(8)) * this->𝜁(𝑓_𝑠)) * pow(𝑒, 2));
    }

    double 𝐴(const double & 𝑒)
    {
        return (log(64) * 2.3) / (0.106 * (𝑒 + 2.3));
    }

    double 𝑑(const double & 𝐿)
    {
        return 7.75 - 5.75 * (pow(((𝐿 * this->a) / 846), 0.41) / (pow(((𝐿 * this->a) / 846), 0.41) + 2));
    }

    double 𝑙(const double & 𝐿)
    {
        return ((M_PI * pow(this->𝑑(𝐿), 2)) / 4) * 𝐿;
    }

    double 𝑠(const double & 𝑒, const double & 𝑓_𝑠)
    {
        return this->𝜁(𝑓_𝑠) * (this->q(0) * pow(𝑒, 2) + this->q(1) * 𝑒) + this->q(2);
    }

    double hatΨ(const double & 𝑒, const double & 𝑓_𝑠, const double & 𝐿)
    {
        return (this->𝑠(𝑒, 𝑓_𝑠) * log10(this->𝑙(𝐿) / this->𝑙_0) + 1) * this->Ψ(𝑒, 𝑓_𝑠);
    }
};

