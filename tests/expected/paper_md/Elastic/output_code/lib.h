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

struct elastic {
    using DT = double;
    struct connection {
        using DT = double;
        Eigen::VectorXd q_g;
        Eigen::VectorXd q_h;
        Eigen::VectorXd m_g;
        Eigen::VectorXd m_h;
        std::function<Eigen::VectorXd(Eigen::VectorXd, Eigen::VectorXd)> angle;
        double λ_q_comma_1;
        double λ_q_comma_2;
        double t;
        long n;
        double E_q;
    
        connection() {}
        connection(const Eigen::VectorXd & q_g, const Eigen::VectorXd & q_h, const Eigen::VectorXd & m_g, const Eigen::VectorXd & m_h, const std::function<Eigen::VectorXd(Eigen::VectorXd, Eigen::VectorXd)> & angle, const double & λ_q_comma_1, const double & λ_q_comma_2, const double & t)
        {
            this->q_g = q_g;
            this->q_h = q_h;
            this->m_g = m_g;
            this->m_h = m_h;
            this->angle = angle;
            this->λ_q_comma_1 = λ_q_comma_1;
            this->λ_q_comma_2 = λ_q_comma_2;
            this->t = t;
            this->n = q_g.rows();
            assert( q_g.rows() == this->n );
            assert( q_h.rows() == this->n );
            assert( m_g.rows() == this->n );
            assert( m_h.rows() == this->n );
            // `E_q` = `$λ_{q,1}$`||`$q_g$`-`$q_h$`+t`$m_g$`||^2 + `$λ_{q,1}$`||`$q_h$`-`$q_g$`+t`$m_h$`||^2 + `$λ_{q,2}$`||`$\angle$`(`$m_g$`,`$m_h$`)||^2
            this->E_q = this->λ_q_comma_1 * pow((this->q_g - this->q_h + this->t * this->m_g).norm(), 2) + this->λ_q_comma_1 * pow((this->q_h - this->q_g + this->t * this->m_h).norm(), 2) + this->λ_q_comma_2 * pow((this->angle(this->m_g, this->m_h)).norm(), 2);
        }
    };
    struct anchor {
        using DT = double;
        double λ_a_comma_1;
        double λ_a_comma_2;
        Eigen::VectorXd q;
        Eigen::VectorXd q_a;
        Eigen::VectorXd m;
        Eigen::VectorXd m_a;
        std::function<Eigen::VectorXd(Eigen::VectorXd, Eigen::VectorXd)> angle;
        long n;
        double E_a;
    
        anchor() {}
        anchor(const double & λ_a_comma_1, const double & λ_a_comma_2, const Eigen::VectorXd & q, const Eigen::VectorXd & q_a, const Eigen::VectorXd & m, const Eigen::VectorXd & m_a, const std::function<Eigen::VectorXd(Eigen::VectorXd, Eigen::VectorXd)> & angle)
        {
            this->λ_a_comma_1 = λ_a_comma_1;
            this->λ_a_comma_2 = λ_a_comma_2;
            this->q = q;
            this->q_a = q_a;
            this->m = m;
            this->m_a = m_a;
            this->angle = angle;
            this->n = q.rows();
            assert( q.rows() == this->n );
            assert( q_a.rows() == this->n );
            assert( m.rows() == this->n );
            assert( m_a.rows() == this->n );
            // `E_a` = `$λ_{a,1}$`||q-`$q_a$`||^2 + `$λ_{a,2}$`||`$\angle$`(m,`$m_a$`)||^2
            this->E_a = this->λ_a_comma_1 * pow((this->q - this->q_a).norm(), 2) + this->λ_a_comma_2 * pow((this->angle(this->m, this->m_a)).norm(), 2);
        }
    };
    struct notchlimit {
        using DT = double;
        double δ_circumflex_accent_minus_sign;
        double δ_circumflex_accent_plus_sign;
        double β_q;
        double β_circumflex_accent_minus_sign;
        double β_circumflex_accent_plus_sign;
        double E_n;
    
        notchlimit() {}
        notchlimit(const double & δ_circumflex_accent_minus_sign, const double & δ_circumflex_accent_plus_sign, const double & β_q, const double & β_circumflex_accent_minus_sign, const double & β_circumflex_accent_plus_sign)
        {
            this->δ_circumflex_accent_minus_sign = δ_circumflex_accent_minus_sign;
            this->δ_circumflex_accent_plus_sign = δ_circumflex_accent_plus_sign;
            this->β_q = β_q;
            this->β_circumflex_accent_minus_sign = β_circumflex_accent_minus_sign;
            this->β_circumflex_accent_plus_sign = β_circumflex_accent_plus_sign;
            // `E_n` = `$δ^{(−)}$`(1/10 log((`$β_q$`-`$β^{(−)}$`)))^2 + `$δ^{(+)}$`(1/10 log((`$β^{(+)}$`-`$β_q$`)))^2
            this->E_n = this->δ_circumflex_accent_minus_sign * pow((double(1) / 10) * log(this->β_q - this->β_circumflex_accent_minus_sign), 2) + this->δ_circumflex_accent_plus_sign * pow((double(1) / 10) * log(this->β_circumflex_accent_plus_sign - this->β_q), 2);
        }
    };
    struct penalty {
        using DT = double;
        double μ;
        double ε;
        double β_q;
        double E_p;
    
        penalty() {}
        penalty(const double & μ, const double & ε, const double & β_q)
        {
            this->μ = μ;
            this->ε = ε;
            this->β_q = β_q;
            // `E_p` = (μ log((ε + `$β_q$`)))^2 + (μ log((ε + 1 - `$β_q$`)))^2
            this->E_p = pow(this->μ * log(this->ε + this->β_q), 2) + pow(this->μ * log(this->ε + 1 - this->β_q), 2);
        }
    };
    connection _connection;
    double E_q;
    anchor _anchor;
    double E_a;
    notchlimit _notchlimit;
    double E_n;
    penalty _penalty;
    double E_p;
    double E_r;
    Eigen::VectorXd q_g;
    Eigen::VectorXd q_h;
    Eigen::VectorXd m_g;
    Eigen::VectorXd m_h;
    std::function<Eigen::VectorXd(Eigen::VectorXd, Eigen::VectorXd)> angle;
    double λ_q_comma_1;
    double λ_q_comma_2;
    double t;
    double λ_a_comma_1;
    double λ_a_comma_2;
    Eigen::VectorXd q;
    Eigen::VectorXd q_a;
    Eigen::VectorXd m;
    Eigen::VectorXd m_a;
    double δ_circumflex_accent_minus_sign;
    double δ_circumflex_accent_plus_sign;
    double β_q;
    double β_circumflex_accent_minus_sign;
    double β_circumflex_accent_plus_sign;
    double μ;
    double ε;
    long n;
    double E;

    elastic() {}
    elastic(const double & E_r, const Eigen::VectorXd & q_g, const Eigen::VectorXd & q_h, const Eigen::VectorXd & m_g, const Eigen::VectorXd & m_h, const std::function<Eigen::VectorXd(Eigen::VectorXd, Eigen::VectorXd)> & angle, const double & λ_q_comma_1, const double & λ_q_comma_2, const double & t, const double & λ_a_comma_1, const double & λ_a_comma_2, const Eigen::VectorXd & q, const Eigen::VectorXd & q_a, const Eigen::VectorXd & m, const Eigen::VectorXd & m_a, const double & δ_circumflex_accent_minus_sign, const double & δ_circumflex_accent_plus_sign, const double & β_q, const double & β_circumflex_accent_minus_sign, const double & β_circumflex_accent_plus_sign, const double & μ, const double & ε)
    {
        this->E_r = E_r;
        this->q_g = q_g;
        this->q_h = q_h;
        this->m_g = m_g;
        this->m_h = m_h;
        this->angle = angle;
        this->λ_q_comma_1 = λ_q_comma_1;
        this->λ_q_comma_2 = λ_q_comma_2;
        this->t = t;
        this->λ_a_comma_1 = λ_a_comma_1;
        this->λ_a_comma_2 = λ_a_comma_2;
        this->q = q;
        this->q_a = q_a;
        this->m = m;
        this->m_a = m_a;
        this->δ_circumflex_accent_minus_sign = δ_circumflex_accent_minus_sign;
        this->δ_circumflex_accent_plus_sign = δ_circumflex_accent_plus_sign;
        this->β_q = β_q;
        this->β_circumflex_accent_minus_sign = β_circumflex_accent_minus_sign;
        this->β_circumflex_accent_plus_sign = β_circumflex_accent_plus_sign;
        this->μ = μ;
        this->ε = ε;
        this->n = q_g.rows();
        assert( q_g.rows() == this->n );
        assert( q_h.rows() == this->n );
        assert( m_g.rows() == this->n );
        assert( m_h.rows() == this->n );
        assert( q.rows() == this->n );
        assert( q_a.rows() == this->n );
        assert( m.rows() == this->n );
        assert( m_a.rows() == this->n );
        this->_connection = connection(this->q_g, this->q_h, this->m_g, this->m_h, this->angle, this->λ_q_comma_1, this->λ_q_comma_2, this->t);
        this->E_q = this->_connection.E_q;
        this->_anchor = anchor(this->λ_a_comma_1, this->λ_a_comma_2, this->q, this->q_a, this->m, this->m_a, this->angle);
        this->E_a = this->_anchor.E_a;
        this->_notchlimit = notchlimit(this->δ_circumflex_accent_minus_sign, this->δ_circumflex_accent_plus_sign, this->β_q, this->β_circumflex_accent_minus_sign, this->β_circumflex_accent_plus_sign);
        this->E_n = this->_notchlimit.E_n;
        this->_penalty = penalty(this->μ, this->ε, this->β_q);
        this->E_p = this->_penalty.E_p;
        // E = `E_r` + `E_q` + `E_a` + `E_n` + `E_p`
        this->E = this->E_r + this->E_q + this->E_a + this->E_n + this->E_p;
    }
};

struct connection {
    using DT = double;
    Eigen::VectorXd q_g;
    Eigen::VectorXd q_h;
    Eigen::VectorXd m_g;
    Eigen::VectorXd m_h;
    std::function<Eigen::VectorXd(Eigen::VectorXd, Eigen::VectorXd)> angle;
    double λ_q_comma_1;
    double λ_q_comma_2;
    double t;
    long n;
    double E_q;

    connection() {}
    connection(const Eigen::VectorXd & q_g, const Eigen::VectorXd & q_h, const Eigen::VectorXd & m_g, const Eigen::VectorXd & m_h, const std::function<Eigen::VectorXd(Eigen::VectorXd, Eigen::VectorXd)> & angle, const double & λ_q_comma_1, const double & λ_q_comma_2, const double & t)
    {
        this->q_g = q_g;
        this->q_h = q_h;
        this->m_g = m_g;
        this->m_h = m_h;
        this->angle = angle;
        this->λ_q_comma_1 = λ_q_comma_1;
        this->λ_q_comma_2 = λ_q_comma_2;
        this->t = t;
        this->n = q_g.rows();
        assert( q_g.rows() == this->n );
        assert( q_h.rows() == this->n );
        assert( m_g.rows() == this->n );
        assert( m_h.rows() == this->n );
        // `E_q` = `$λ_{q,1}$`||`$q_g$`-`$q_h$`+t`$m_g$`||^2 + `$λ_{q,1}$`||`$q_h$`-`$q_g$`+t`$m_h$`||^2 + `$λ_{q,2}$`||`$\angle$`(`$m_g$`,`$m_h$`)||^2
        this->E_q = this->λ_q_comma_1 * pow((this->q_g - this->q_h + this->t * this->m_g).norm(), 2) + this->λ_q_comma_1 * pow((this->q_h - this->q_g + this->t * this->m_h).norm(), 2) + this->λ_q_comma_2 * pow((this->angle(this->m_g, this->m_h)).norm(), 2);
    }
};

struct anchor {
    using DT = double;
    double λ_a_comma_1;
    double λ_a_comma_2;
    Eigen::VectorXd q;
    Eigen::VectorXd q_a;
    Eigen::VectorXd m;
    Eigen::VectorXd m_a;
    std::function<Eigen::VectorXd(Eigen::VectorXd, Eigen::VectorXd)> angle;
    long n;
    double E_a;

    anchor() {}
    anchor(const double & λ_a_comma_1, const double & λ_a_comma_2, const Eigen::VectorXd & q, const Eigen::VectorXd & q_a, const Eigen::VectorXd & m, const Eigen::VectorXd & m_a, const std::function<Eigen::VectorXd(Eigen::VectorXd, Eigen::VectorXd)> & angle)
    {
        this->λ_a_comma_1 = λ_a_comma_1;
        this->λ_a_comma_2 = λ_a_comma_2;
        this->q = q;
        this->q_a = q_a;
        this->m = m;
        this->m_a = m_a;
        this->angle = angle;
        this->n = q.rows();
        assert( q.rows() == this->n );
        assert( q_a.rows() == this->n );
        assert( m.rows() == this->n );
        assert( m_a.rows() == this->n );
        // `E_a` = `$λ_{a,1}$`||q-`$q_a$`||^2 + `$λ_{a,2}$`||`$\angle$`(m,`$m_a$`)||^2
        this->E_a = this->λ_a_comma_1 * pow((this->q - this->q_a).norm(), 2) + this->λ_a_comma_2 * pow((this->angle(this->m, this->m_a)).norm(), 2);
    }
};

struct notchlimit {
    using DT = double;
    double δ_circumflex_accent_minus_sign;
    double δ_circumflex_accent_plus_sign;
    double β_q;
    double β_circumflex_accent_minus_sign;
    double β_circumflex_accent_plus_sign;
    double E_n;

    notchlimit() {}
    notchlimit(const double & δ_circumflex_accent_minus_sign, const double & δ_circumflex_accent_plus_sign, const double & β_q, const double & β_circumflex_accent_minus_sign, const double & β_circumflex_accent_plus_sign)
    {
        this->δ_circumflex_accent_minus_sign = δ_circumflex_accent_minus_sign;
        this->δ_circumflex_accent_plus_sign = δ_circumflex_accent_plus_sign;
        this->β_q = β_q;
        this->β_circumflex_accent_minus_sign = β_circumflex_accent_minus_sign;
        this->β_circumflex_accent_plus_sign = β_circumflex_accent_plus_sign;
        // `E_n` = `$δ^{(−)}$`(1/10 log((`$β_q$`-`$β^{(−)}$`)))^2 + `$δ^{(+)}$`(1/10 log((`$β^{(+)}$`-`$β_q$`)))^2
        this->E_n = this->δ_circumflex_accent_minus_sign * pow((double(1) / 10) * log(this->β_q - this->β_circumflex_accent_minus_sign), 2) + this->δ_circumflex_accent_plus_sign * pow((double(1) / 10) * log(this->β_circumflex_accent_plus_sign - this->β_q), 2);
    }
};

struct penalty {
    using DT = double;
    double μ;
    double ε;
    double β_q;
    double E_p;

    penalty() {}
    penalty(const double & μ, const double & ε, const double & β_q)
    {
        this->μ = μ;
        this->ε = ε;
        this->β_q = β_q;
        // `E_p` = (μ log((ε + `$β_q$`)))^2 + (μ log((ε + 1 - `$β_q$`)))^2
        this->E_p = pow(this->μ * log(this->ε + this->β_q), 2) + pow(this->μ * log(this->ε + 1 - this->β_q), 2);
    }
};

