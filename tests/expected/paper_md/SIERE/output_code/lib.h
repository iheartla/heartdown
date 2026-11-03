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
#ifndef IHEARTLA_SORTED_SET
#define IHEARTLA_SORTED_SET
// sets are sorted vectors without duplicates (defined before the last #include:
// code that wraps the generated code in a namespace starts it after that)
template<typename T> std::vector<T> sorted_set(std::vector<T> v) { std::sort(v.begin(), v.end()); v.erase(std::unique(v.begin(), v.end()), v.end()); return v; }
// (vectors in lexicographic order)
template<typename S, int R, int C, int O, int MR, int MC> std::vector<Eigen::Matrix<S, R, C, O, MR, MC>> sorted_set(std::vector<Eigen::Matrix<S, R, C, O, MR, MC>> v) {
using M = Eigen::Matrix<S, R, C, O, MR, MC>;
std::sort(v.begin(), v.end(), [](const M & a, const M & b){ return std::lexicographical_compare(a.data(), a.data() + a.size(), b.data(), b.data() + b.size()); });
v.erase(std::unique(v.begin(), v.end(), [](const M & a, const M & b){ return a.size() == b.size() && (a.array() == b.array()).all(); }), v.end());
return v;
}
#endif
#include <unsupported/Eigen/MatrixFunctions>

struct siere {
    using DT = double;
    Eigen::MatrixXd U_s;
    Eigen::MatrixXd M;
    Eigen::VectorXd v;
    Eigen::VectorXd f;
    Eigen::MatrixXd K;
    Eigen::MatrixXd boldsymbolI;
    double h;
    std::function<Eigen::MatrixXd(Eigen::MatrixXd)> φ_1;
    Eigen::MatrixXd u;
    long n;
    long s;
    Eigen::VectorXd v_G;
    Eigen::VectorXd v_H;
    Eigen::VectorXd f_G;
    Eigen::MatrixXd Gu;
    Eigen::VectorXd f_H;
    Eigen::MatrixXd Hu;
    Eigen::MatrixXd J_G;
    Eigen::MatrixXd J_H;
    Eigen::MatrixXd J_G_circumflex_accent_r;
    Eigen::MatrixXd G_circumflex_accent_ru;
    Eigen::MatrixXd u_plus_sign;

    siere() {}
    siere(const Eigen::MatrixXd & U_s, const Eigen::MatrixXd & M, const Eigen::VectorXd & v, const Eigen::VectorXd & f, const Eigen::MatrixXd & K, const Eigen::MatrixXd & boldsymbolI, const double & h, const std::function<Eigen::MatrixXd(Eigen::MatrixXd)> & φ_1, const Eigen::MatrixXd & u)
    {
        this->U_s = U_s;
        this->M = M;
        this->v = v;
        this->f = f;
        this->K = K;
        this->boldsymbolI = boldsymbolI;
        this->h = h;
        this->φ_1 = φ_1;
        this->u = u;
        this->n = U_s.rows();
        this->s = U_s.cols();
        assert( U_s.rows() == this->n && U_s.cols() == this->s );
        assert( M.rows() == this->n && M.cols() == this->n );
        assert( v.rows() == this->n );
        assert( f.rows() == this->n );
        assert( K.rows() == this->n && K.cols() == this->n );
        assert( boldsymbolI.rows() == 2*this->n && boldsymbolI.cols() == 2*this->n );
        assert( u.rows() == 2*this->n && u.cols() == 1 );
        // `$v_G$` = `$U_s$``$U_s$`^T Mv
        this->v_G = ((this->U_s * this->U_s.transpose()) * this->M) * this->v;
        // `$v_H$` =  v - `$v_G$`
        this->v_H = this->v - this->v_G;
        // `$f_G$` =  M`$U_s$``$U_s$`^T f
        this->f_G = ((this->M * this->U_s) * this->U_s.transpose()) * this->f;
        // `$G(u)$` = [`$v_G$`
        //           M⁻¹`$f_G$`]
        this->Gu = [&](){ Eigen::MatrixXd m(2*this->n, 1); m << this->v_G, ((this->M).eval()).colPivHouseholderQr().solve((this->f_G).eval()); return m; }();
        // `$f_H$` =  f - `$f_G$`
        this->f_H = this->f - this->f_G;
        // `$H(u)$` = [`$v_H$`
        //           M⁻¹`$f_H$`]
        this->Hu = [&](){ Eigen::MatrixXd m(2*this->n, 1); m << this->v_H, ((this->M).eval()).colPivHouseholderQr().solve((this->f_H).eval()); return m; }();
        // `$J_G$` = [0    `$U_s$``$U_s$`^TM
        //       -`$U_s$``$U_s$`^TK`$U_s$``$U_s$`^TM 0 ]
        this->J_G = [&](){ Eigen::MatrixXd m(2*this->n, 2*this->n); m << Eigen::MatrixXd::Zero(this->n, this->n), (this->U_s * this->U_s.transpose()) * this->M, -(((((this->U_s * this->U_s.transpose()) * this->K) * this->U_s) * this->U_s.transpose()) * this->M), Eigen::MatrixXd::Zero(this->n, this->n); return m; }();
        // `$J_H$` =  [0     I_n
        //             -M⁻¹K 0] - `$J_G$`
        this->J_H = [&](){ Eigen::MatrixXd m(2*this->n, 2*this->n); m << Eigen::MatrixXd::Zero(this->n, this->n), Eigen::MatrixXd::Identity(this->n, this->n), -((this->M).eval()).colPivHouseholderQr().solve((this->K).eval()), Eigen::MatrixXd::Zero(this->n, this->n); return m; }() - this->J_G;
        // `$J_G^r$` = [0     I_s
        //             -`$U_s$`^TK`$U_s$` 0]
        this->J_G_circumflex_accent_r = [&](){ Eigen::MatrixXd m(2*this->s, 2*this->s); m << Eigen::MatrixXd::Zero(this->s, this->s), Eigen::MatrixXd::Identity(this->s, this->s), -((this->U_s.transpose() * this->K) * this->U_s), Eigen::MatrixXd::Zero(this->s, this->s); return m; }();
        // `$G^r(u)$` = [`$U_s$`^TMv
        //             `$U_s$`^Tf]
        this->G_circumflex_accent_ru = [&](){ Eigen::MatrixXd m(2*this->s, 1); m << (this->U_s.transpose() * this->M) * this->v, this->U_s.transpose() * this->f; return m; }();
        // `$u_+$` =  u + (`$\boldsymbol{I}$` -h`$J_H$`)⁻¹(h `$H(u)$` + h[`$U_s$` 0
        //                                                0   `$U_s$`] `$φ_1$`(h`$J_G^r$`) `$G^r(u)$`)
        this->u_plus_sign = this->u + ((this->boldsymbolI - this->h * this->J_H).eval()).colPivHouseholderQr().solve((this->h * this->Hu + ((this->h * [&](){ Eigen::MatrixXd m(2*this->n, 2*this->s); m << this->U_s, Eigen::MatrixXd::Zero(this->n, this->s), Eigen::MatrixXd::Zero(this->n, this->s), this->U_s; return m; }()) * this->φ_1(this->h * this->J_G_circumflex_accent_r)) * this->G_circumflex_accent_ru).eval());
    }
};

struct second {
    using DT = double;
    Eigen::MatrixXd U_s;
    Eigen::MatrixXd M;
    Eigen::MatrixXd K;
    long n;
    long s;
    Eigen::MatrixXd Y_1;
    Eigen::MatrixXd Z_1;
    Eigen::MatrixXd Y_2;
    Eigen::MatrixXd Z_2;
    Eigen::MatrixXd J_G;

    second() {}
    second(const Eigen::MatrixXd & U_s, const Eigen::MatrixXd & M, const Eigen::MatrixXd & K)
    {
        this->U_s = U_s;
        this->M = M;
        this->K = K;
        this->n = U_s.rows();
        this->s = U_s.cols();
        assert( U_s.rows() == this->n && U_s.cols() == this->s );
        assert( M.rows() == this->n && M.cols() == this->n );
        assert( K.rows() == this->n && K.cols() == this->n );
        // `$Y_1$` =  [`$U_s$`
        //              0]
        this->Y_1 = [&](){ Eigen::MatrixXd m(this->n + 1, this->s); m << this->U_s, Eigen::MatrixXd::Zero(1, this->s); return m; }();
        // `$Z_1$` =  [ 0
        //              M`$U_s$`]
        this->Z_1 = [&](){ Eigen::MatrixXd m(this->n + 1, this->s); m << Eigen::MatrixXd::Zero(1, this->s), this->M * this->U_s; return m; }();
        // `$Y_2$` =  [0
        //             -`$U_s$``$U_s$`^TK`$U_s$`]
        this->Y_2 = [&](){ Eigen::MatrixXd m(this->n + 1, this->s); m << Eigen::MatrixXd::Zero(1, this->s), -(((this->U_s * this->U_s.transpose()) * this->K) * this->U_s); return m; }();
        // `$Z_2$` =  [ M`$U_s$`
        //              0]
        this->Z_2 = [&](){ Eigen::MatrixXd m(this->n + 1, this->s); m << this->M * this->U_s, Eigen::MatrixXd::Zero(1, this->s); return m; }();
        // `$J_G$` = `$Y_1$``$Z_1$`^T + `$Y_2$``$Z_2$`^T
        this->J_G = this->Y_1 * this->Z_1.transpose() + this->Y_2 * this->Z_2.transpose();
    }
};

