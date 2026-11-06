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

struct Generic {
    using DT = double;
    Eigen::Matrix<double, 2, 2> J;
    double x;
    double y;
    double u;
    double v;
    Eigen::Matrix<double, 2, 1> x_p;
    Eigen::Matrix<double, 2, 1> v_p;
    Eigen::Matrix<double, 2, 6> M;

    Generic(const Eigen::Matrix<double, 2, 2> & J, const double & x, const double & y, const double & u, const double & v)
    {
        this->J = J;
        this->x = x;
        this->y = y;
        this->u = u;
        this->v = v;
        // `$x_p$` = (-y, x)
        this->x_p = [&](){ Eigen::Matrix<double, 2, 1> v(2); v << -this->y, this->x; return v; }();
        // `$v_p$` = (-v, u)
        this->v_p = [&](){ Eigen::Matrix<double, 2, 1> v(2); v << -this->v, this->u; return v; }();
        // M=[-J`$x_p$`+`$v_p$`  J  `$x_p$`  I_2]
        this->M = [&](){ Eigen::MatrixXd m(2, 6); m << -(this->J * this->x_p) + this->v_p, this->J, this->x_p, Eigen::Matrix<double, 2, 2>::Identity(2, 2); return m; }();
    }
};

