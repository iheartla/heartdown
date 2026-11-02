#pragma once
#include <Eigen/Core>
#include <Eigen/QR>
#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <iostream>
#include <set>
#include <algorithm>
#include "type_helper.h"

struct Anisotropic {
    using DT = double;
    using MatrixD = Eigen::MatrixXd;
    using VectorD = Eigen::VectorXd;
    Eigen::Matrix<DT, 3, 3> A;
    DT lambda_0_comma_1_comma_2;
    DT I_5;
    Eigen::Matrix<DT, 3, 3> fracdeeI5deeF;
    Eigen::Matrix<DT, 9, 9> fracdee2I5deef2;
    Eigen::Matrix<DT, 3, 3> mathbfQ_0;
    Eigen::Matrix<DT, 3, 3> mathbfQ_1;
    Eigen::Matrix<DT, 3, 3> mathbfQ_2;

    Anisotropic(
        const Eigen::Matrix<double, 3, 1> & a,
        const Eigen::Matrix<double, 3, 3> & C,
        const Eigen::Matrix<double, 3, 3> & F)
    {
    
        // A = a a^T
        A = a * a.transpose();
        // `$\lambda_{0,1,2}$`=2||a||_2^2
        lambda_0_comma_1_comma_2 = 2 * pow((a).template lpNorm<2>(), 2);
        // `$I_5$` = tr(CA)
        I_5 = (C * A).trace();
        // `$\frac{∂I₅}{∂F}$` = 2FA
        fracdeeI5deeF = DT(2) * F * A;
        // `$\frac{∂²I₅}{∂f²}$` = 2[A₁,₁I₃  A₁,₂I₃  A₁,₃I₃
        //                A₂,₁I₃  A₂,₂I₃  A₂,₃I₃
        //                A₃,₁I₃  A₃,₂I₃  A₃,₃I₃]
        Eigen::Matrix<DT, 9, 9> fracdee2I5deef2_0;
        fracdee2I5deef2_0 << A(1-1, 1-1) * MatrixD::Identity(3, 3), A(1-1, 2-1) * MatrixD::Identity(3, 3), A(1-1, 3-1) * MatrixD::Identity(3, 3),
        A(2-1, 1-1) * MatrixD::Identity(3, 3), A(2-1, 2-1) * MatrixD::Identity(3, 3), A(2-1, 3-1) * MatrixD::Identity(3, 3),
        A(3-1, 1-1) * MatrixD::Identity(3, 3), A(3-1, 2-1) * MatrixD::Identity(3, 3), A(3-1, 3-1) * MatrixD::Identity(3, 3);
        fracdee2I5deef2 = DT(2) * fracdee2I5deef2_0;
        // `$\mathbf{Q}_{0}$` = [a^T
        //                0
        //                0]
        Eigen::Matrix<DT, 3, 3> mathbfQ_0_0;
        mathbfQ_0_0 << a.transpose(),
        MatrixD::Zero(1, 3),
        MatrixD::Zero(1, 3);
        mathbfQ_0 = mathbfQ_0_0;
        // `$\mathbf{Q}_{1}$` = [0
        //                     a^T
        //                0]
        Eigen::Matrix<DT, 3, 3> mathbfQ_1_0;
        mathbfQ_1_0 << MatrixD::Zero(1, 3),
        a.transpose(),
        MatrixD::Zero(1, 3);
        mathbfQ_1 = mathbfQ_1_0;
        // `$\mathbf{Q}_{2}$` = [ 0
        //                0
        //                a^T]
        Eigen::Matrix<DT, 3, 3> mathbfQ_2_0;
        mathbfQ_2_0 << MatrixD::Zero(1, 3),
        MatrixD::Zero(1, 3),
        a.transpose();
        mathbfQ_2 = mathbfQ_2_0;
    }
};

struct Anisotropic2D {
    using DT = double;
    using MatrixD = Eigen::MatrixXd;
    using VectorD = Eigen::VectorXd;
    DT lambda_0_comma_1;
    Eigen::Matrix<DT, 4, 4> fracdee2I5deef2;
    Eigen::Matrix<DT, 2, 2> mathbfQ_0;
    Eigen::Matrix<DT, 2, 2> mathbfQ_1;

    Anisotropic2D(
        const Eigen::Matrix<double, 2, 1> & a,
        const Eigen::Matrix<double, 2, 2> & A)
    {
    
        // `$\lambda_{0,1}$`=2||a||_2^2
        lambda_0_comma_1 = 2 * pow((a).template lpNorm<2>(), 2);
        // `$\frac{∂²I₅}{∂f²}$` = 2[A₁,₁I_2  A₁,₂I_2
        //                A₂,₁I_2  A₂,₂I_2]
        Eigen::Matrix<DT, 4, 4> fracdee2I5deef2_0;
        fracdee2I5deef2_0 << A(1-1, 1-1) * MatrixD::Identity(2, 2), A(1-1, 2-1) * MatrixD::Identity(2, 2),
        A(2-1, 1-1) * MatrixD::Identity(2, 2), A(2-1, 2-1) * MatrixD::Identity(2, 2);
        fracdee2I5deef2 = DT(2) * fracdee2I5deef2_0;
        // `$\mathbf{Q}_{0}$` = [a^T
        //                0]
        Eigen::Matrix<DT, 2, 2> mathbfQ_0_0;
        mathbfQ_0_0 << a.transpose(),
        MatrixD::Zero(1, 2);
        mathbfQ_0 = mathbfQ_0_0;
        // `$\mathbf{Q}_{1}$` = [0
        //                     a^T]
        Eigen::Matrix<DT, 2, 2> mathbfQ_1_0;
        mathbfQ_1_0 << MatrixD::Zero(1, 2),
        a.transpose();
        mathbfQ_1 = mathbfQ_1_0;
    }
};

