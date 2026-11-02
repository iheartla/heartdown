#pragma once
#include <Eigen/Core>
#include <Eigen/QR>
#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <iostream>
#include <set>
#include <algorithm>
#include "type_helper.h"

struct Generic {
    using DT = double;
    using MatrixD = Eigen::MatrixXd;
    using VectorD = Eigen::VectorXd;
    Eigen::Matrix<DT, 2, 1> x_p;
    Eigen::Matrix<DT, 2, 1> v_p;
    Eigen::Matrix<DT, 2, 6> M;

    Generic(
        const Eigen::Matrix<double, 2, 2> & J,
        const double & x,
        const double & y,
        const double & u,
        const double & v)
    {
    
        // `$x_p$` = (-y, x)
        Eigen::Matrix<DT, 2, 1> x_p_0;
        x_p_0 << -y, x;
        x_p = x_p_0;
        // `$v_p$` = (-v, u)
        Eigen::Matrix<DT, 2, 1> v_p_0;
        v_p_0 << -v, u;
        v_p = v_p_0;
        // M=[-J`$x_p$`+`$v_p$`  J  `$x_p$`  I_2]
        Eigen::Matrix<DT, 2, 6> M_0;
        M_0 << -J * x_p + v_p, J, x_p, MatrixD::Identity(2, 2);
        M = M_0;
    }
};

