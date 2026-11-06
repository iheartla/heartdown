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

struct clustering {
    using DT = double;
    std::vector<Eigen::VectorXd> y;
    long y_n;
    long m;
    double ret;

    clustering(const std::vector<Eigen::VectorXd> & y)
    {
        this->y = y;
        this->y_n = y.size();
        this->m = y[0].rows();
        // min_( c ∈ ℝ^m ) ∑_i ‖ y_i - c ‖^2
        this->ret = [&]() -> double { const int N = 0 + (this->m)*(1); auto objective = [&](const Eigen::VectorXd & X) -> double { const Eigen::VectorXd c = Eigen::Map<const Eigen::MatrixXd>(X.data() + (0), this->m, 1); return (double([&](){ double sum = 0.0; for(int i = 1; i <= this->y_n; i++){ sum += pow((this->y.at(i-1) - c).norm(), 2); } return sum; }())); }; double best_f = std::numeric_limits<double>::infinity(); Eigen::VectorXd best_x; auto f = [&](const Eigen::VectorXd & X, Eigen::VectorXd & grad) -> double { Eigen::VectorXd Y = X; for(int k = 0; k < X.size(); k++){ const double h = 1e-6 * std::max(1.0, std::abs(X(k))); Y(k) = X(k) + h; const double fp = objective(Y); Y(k) = X(k) - h; const double fm = objective(Y); Y(k) = X(k); grad(k) = (fp - fm) / (2 * h); } const double value = objective(X); if(value < best_f){ best_f = value; best_x = X; } return value; }; Eigen::VectorXd x = Eigen::VectorXd::Zero(N); LBFGSpp::LBFGSParam<double> param; param.epsilon = 1e-10; param.max_iterations = 1000; LBFGSpp::LBFGSSolver<double> solver(param); double fx; try { solver.minimize(f, x, fx); } catch(const std::runtime_error &) { if(best_x.size() == 0) throw; x = best_x; fx = best_f; } return fx; }();
    }
};

