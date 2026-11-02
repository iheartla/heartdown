#pragma once
#include <Eigen/Core>
#include <Eigen/QR>
#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <iostream>
#include <set>
#include <algorithm>
#include "type_helper.h"

struct siere {
    using DT = double;
    using MatrixD = Eigen::MatrixXd;
    using VectorD = Eigen::VectorXd;
    Eigen::Matrix<DT, Eigen::Dynamic, 1> Gu;
    Eigen::Matrix<DT, Eigen::Dynamic, 1> Hu;
    VectorD v_G;
    VectorD v_H;
    VectorD f_G;
    VectorD f_H;
    MatrixD J_G;
    MatrixD J_H;
    Eigen::Matrix<DT, Eigen::Dynamic, 1> u_plus_sign;
    MatrixD J_G_circumflex_accent_r;
    Eigen::Matrix<DT, Eigen::Dynamic, 1> G_circumflex_accent_ru;

    siere(
        const Eigen::MatrixXd & U_s,
        const Eigen::MatrixXd & M,
        const Eigen::VectorXd & v,
        const Eigen::VectorXd & f,
        const Eigen::MatrixXd & K,
        const Eigen::MatrixXd & boldsymbolI,
        const double & h,
        const std::function<Eigen::MatrixXd(Eigen::MatrixXd)> & φ_1,
        const Eigen::Matrix<double, Eigen::Dynamic, 1> & u)
    {
        const long n = U_s.rows();
        const long s = U_s.cols();
        assert( U_s.rows() == n );
        assert( M.rows() == n );
        assert( M.cols() == n );
        assert( v.size() == n );
        assert( f.size() == n );
        assert( K.rows() == n );
        assert( K.cols() == n );
        assert( boldsymbolI.rows() == 2*n );
        assert( boldsymbolI.cols() == 2*n );
        assert( u.rows() == 2*n );
        assert( u.cols() == 1 );
        assert( fmod(2*n, 1) == 0.0 );
        assert( fmod(2*n, 1) == 0.0 );
        assert( fmod(2*n, 1) == 0.0 );
        // `$v_G$` = `$U_s$``$U_s$`^T Mv
        v_G = U_s * U_s.transpose() * M * v;
        // `$v_H$` =  v - `$v_G$`
        v_H = v - v_G;
        // `$f_G$` =  M`$U_s$``$U_s$`^T f
        f_G = M * U_s * U_s.transpose() * f;
        // `$G(u)$` = [`$v_G$`
        //           M⁻¹`$f_G$`]
        MatrixD Gu_0(2*n, 1);
        Gu_0 << v_G,
        (to_double(M)).colPivHouseholderQr().solve(to_double(f_G));
        Gu = Gu_0;
        // `$f_H$` =  f - `$f_G$`
        f_H = f - f_G;
        // `$H(u)$` = [`$v_H$`
        //           M⁻¹`$f_H$`]
        MatrixD Hu_0(2*n, 1);
        Hu_0 << v_H,
        (to_double(M)).colPivHouseholderQr().solve(to_double(f_H));
        Hu = Hu_0;
        // `$J_G$` = [0    `$U_s$``$U_s$`^TM
        //       -`$U_s$``$U_s$`^TK`$U_s$``$U_s$`^TM 0 ]
        MatrixD J_G_0(2*n, 2*n);
        J_G_0 << MatrixD::Zero(n, n), U_s * U_s.transpose() * M,
        -U_s * U_s.transpose() * K * U_s * U_s.transpose() * M, MatrixD::Zero(n, n);
        J_G = J_G_0;
        // `$J_H$` =  [0     I_n
        //             -M⁻¹K 0] - `$J_G$`
        MatrixD J_H_0(2*n, 2*n);
        J_H_0 << MatrixD::Zero(n, n), MatrixD::Identity(n, n),
        -(to_double(M)).colPivHouseholderQr().solve(to_double(K)), MatrixD::Zero(n, n);
        J_H = J_H_0 - J_G;
        // `$J_G^r$` = [0     I_s
        //             -`$U_s$`^TK`$U_s$` 0]
        MatrixD J_G_circumflex_accent_r_0(2*s, 2*s);
        J_G_circumflex_accent_r_0 << MatrixD::Zero(s, s), MatrixD::Identity(s, s),
        -U_s.transpose() * K * U_s, MatrixD::Zero(s, s);
        J_G_circumflex_accent_r = J_G_circumflex_accent_r_0;
        // `$G^r(u)$` = [`$U_s$`^TMv
        //             `$U_s$`^Tf]
        MatrixD G_circumflex_accent_ru_0(2*s, 1);
        G_circumflex_accent_ru_0 << U_s.transpose() * M * v,
        U_s.transpose() * f;
        G_circumflex_accent_ru = G_circumflex_accent_ru_0;
        // `$u_+$` =  u + (`$\boldsymbol{I}$` -h`$J_H$`)⁻¹(h `$H(u)$` + h[`$U_s$` 0
        //                                                0   `$U_s$`] `$φ_1$`(h`$J_G^r$`) `$G^r(u)$`)
        MatrixD u_plus_sign_2(2*n, 2*s);
        u_plus_sign_2 << U_s, MatrixD::Zero(n, s),
        MatrixD::Zero(n, s), U_s;
        u_plus_sign = u + (to_double((boldsymbolI - h * J_H))).colPivHouseholderQr().solve(to_double((h * Hu + h * u_plus_sign_2 * φ_1((h * J_G_circumflex_accent_r).eval()) * G_circumflex_accent_ru)));
    }
};

struct second {
    using DT = double;
    using MatrixD = Eigen::MatrixXd;
    using VectorD = Eigen::VectorXd;
    MatrixD J_G;
    MatrixD Y_1;
    MatrixD Z_1;
    MatrixD Y_2;
    MatrixD Z_2;

    second(
        const Eigen::MatrixXd & U_s,
        const Eigen::MatrixXd & M,
        const Eigen::MatrixXd & K)
    {
        const long n = U_s.rows();
        const long s = U_s.cols();
        assert( U_s.rows() == n );
        assert( M.rows() == n );
        assert( M.cols() == n );
        assert( K.rows() == n );
        assert( K.cols() == n );
        // `$Y_1$` =  [`$U_s$`
        //              0]
        MatrixD Y_1_0(n + 1, s);
        Y_1_0 << U_s,
        MatrixD::Zero(1, s);
        Y_1 = Y_1_0;
        // `$Z_1$` =  [ 0
        //              M`$U_s$`]
        MatrixD Z_1_0(n + 1, s);
        Z_1_0 << MatrixD::Zero(1, s),
        M * U_s;
        Z_1 = Z_1_0;
        // `$Y_2$` =  [0
        //             -`$U_s$``$U_s$`^TK`$U_s$`]
        MatrixD Y_2_0(n + 1, s);
        Y_2_0 << MatrixD::Zero(1, s),
        -U_s * U_s.transpose() * K * U_s;
        Y_2 = Y_2_0;
        // `$Z_2$` =  [ M`$U_s$`
        //              0]
        MatrixD Z_2_0(n + 1, s);
        Z_2_0 << M * U_s,
        MatrixD::Zero(1, s);
        Z_2 = Z_2_0;
        // `$J_G$` = `$Y_1$``$Z_1$`^T + `$Y_2$``$Z_2$`^T
        J_G = Y_1 * Z_1.transpose() + Y_2 * Z_2.transpose();
    }
};

