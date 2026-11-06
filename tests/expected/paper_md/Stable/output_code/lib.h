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

struct Stable {
    using DT = double;
    Eigen::Matrix<double, 3, 3> F;
    double μ;
    double I_C;
    double λ;
    Eigen::Matrix<double, 3, 1> f_0;
    Eigen::Matrix<double, 3, 1> f_1;
    Eigen::Matrix<double, 3, 1> f_2;
    double α;
    double J;
    Eigen::Matrix<double, 3, 3> fracpartialJpartialF;
    Eigen::Matrix<double, 3, 3> PF;

    Stable(const Eigen::Matrix<double, 3, 3> & F, const double & μ, const double & I_C, const double & λ, const Eigen::Matrix<double, 3, 1> & f_0, const Eigen::Matrix<double, 3, 1> & f_1, const Eigen::Matrix<double, 3, 1> & f_2)
    {
        this->F = F;
        this->μ = μ;
        this->I_C = I_C;
        this->λ = λ;
        this->f_0 = f_0;
        this->f_1 = f_1;
        this->f_2 = f_2;
        // α = 1 + μ/λ - μ/(4λ)
        this->α = 1 + this->μ / this->λ - this->μ / (4 * this->λ);
        // J =`$f_0$`⋅(`$f_1$`×`$f_2$`)
        this->J = ((this->f_0).eval()).dot(((((this->f_1).eval()).cross((this->f_2).eval()))).eval());
        // `$\frac{\partial J}{\partial F}$` = [`$f_1$`×`$f_2$` `$f_2$`×`$f_0$` `$f_0$`×`$f_1$`]
        this->fracpartialJpartialF = [&](){ Eigen::MatrixXd m(3, 3); m << ((this->f_1).eval()).cross((this->f_2).eval()), ((this->f_2).eval()).cross((this->f_0).eval()), ((this->f_0).eval()).cross((this->f_1).eval()); return m; }();
        // `$P(F)$` = μ(1-1/(`$I_C$`+1))F + λ(J- α)`$\frac{\partial J}{\partial F}$`
        this->PF = (this->μ * (1 - 1 / (this->I_C + 1))) * this->F + (this->λ * (this->J - this->α)) * this->fracpartialJpartialF;
    }
};

