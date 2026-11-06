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

#include <LBFGS.h>
#include <LBFGSB.h>

struct MeshSmoothing {
    using DT = double;
    Eigen::MatrixXd V;
    Eigen::SparseMatrix<double> L;
    double λ;
    long n;
    double ret;

    MeshSmoothing(const Eigen::MatrixXd & V, const Eigen::SparseMatrix<double> & L, const double & λ)
    {
        this->V = V;
        this->L = L;
        this->λ = λ;
        this->n = V.rows();
        assert( V.rows() == this->n && V.cols() == 3 );
        // min_( U ∈ ℝ^(n×3) ) (1-λ) `E_\text{data}`(U) + λ `E_\text{smoothness}`(U)
        this->ret = [&]() -> double { const int N = 0 + (this->n)*(3); auto objective = [&](const Eigen::VectorXd & X) -> double { const Eigen::MatrixXd U = Eigen::Map<const Eigen::MatrixXd>(X.data() + (0), this->n, 3); return (double((1 - this->λ) * this->E_textdata(U) + this->λ * this->E_textsmoothness(U))); }; double best_f = std::numeric_limits<double>::infinity(); Eigen::VectorXd best_x; auto f = [&](const Eigen::VectorXd & X, Eigen::VectorXd & grad) -> double { Eigen::VectorXd Y = X; for(int k = 0; k < X.size(); k++){ const double h = 1e-6 * std::max(1.0, std::abs(X(k))); Y(k) = X(k) + h; const double fp = objective(Y); Y(k) = X(k) - h; const double fm = objective(Y); Y(k) = X(k); grad(k) = (fp - fm) / (2 * h); } const double value = objective(X); if(value < best_f){ best_f = value; best_x = X; } return value; }; Eigen::VectorXd x = Eigen::VectorXd::Zero(N); LBFGSpp::LBFGSParam<double> param; param.epsilon = 1e-10; param.max_iterations = 1000; LBFGSpp::LBFGSSolver<double> solver(param); double fx; try { solver.minimize(f, x, fx); } catch(const std::runtime_error &) { if(best_x.size() == 0) throw; x = best_x; fx = best_f; } return fx; }();
    }

    double E_textdata(const Eigen::MatrixXd & U)
    {
        return pow((U - this->V).norm(), 2);
    }

    double E_textsmoothness(const Eigen::MatrixXd & U)
    {
        return ((U.transpose() * this->L) * U).trace();
    }
};

