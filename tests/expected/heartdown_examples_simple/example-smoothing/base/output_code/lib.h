#pragma once
#include <Eigen/Core>
#include <Eigen/QR>
#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <iostream>
#include <set>
#include <algorithm>
#include <LBFGS.h>
#include <LBFGSB.h>
#include <limits>
#include <stdexcept>
#include "type_helper.h"

struct MeshSmoothing {
    using DT = double;
    using MatrixD = Eigen::MatrixXd;
    using VectorD = Eigen::VectorXd;
    DT ret;
    Eigen::Matrix<double, Eigen::Dynamic, 3> V;
    Eigen::SparseMatrix<double> L;
    double λ;
    template<typename REAL>
    REAL E_textdata(
        const Eigen::Matrix<REAL, Eigen::Dynamic, 3> & U)
    {
        const long n = U.rows();
        assert( U.cols() == 3 );

        return pow((U - this->V).norm(), 2);    
    }
    template<typename REAL>
    REAL E_textsmoothness(
        const Eigen::Matrix<REAL, Eigen::Dynamic, 3> & U)
    {
        const long n = U.rows();
        assert( U.cols() == 3 );

        return (U.transpose() * this->L * U).trace();    
    }
    MeshSmoothing(
        const Eigen::Matrix<double, Eigen::Dynamic, 3> & V,
        const Eigen::SparseMatrix<double> & L,
        const double & λ)
    {
        const long n = V.rows();
        assert( V.cols() == 3 );
        assert( L.rows() == n );
        assert( L.cols() == n );
        this->V = V;
        this->L = L;
        this->λ = λ;
        auto unpack = [&](const VectorD & X_1, Eigen::Matrix<DT, Eigen::Dynamic, 3> & U)
        {
            U = Eigen::Map<const Eigen::Matrix<DT, Eigen::Dynamic, 3>>(X_1.data() + 0, n, 3);
        };
        auto target_1 = [&](const VectorD & X_1)
        {
            Eigen::Matrix<DT, Eigen::Dynamic, 3> U;
            unpack(X_1, U);
            return DT(E_textdata((U).eval()) + λ * E_textsmoothness((U).eval()));
        };
        VectorD best_X;
        DT best_f = std::numeric_limits<DT>::infinity();
        auto helper = [&](const VectorD & X_1, VectorD & grad)
        {
            VectorD Xh = X_1;
            for(int k = 0; k < X_1.size(); ++k)
            {
                const DT h = 1e-6 * std::max(DT(1), std::abs(X_1(k)));
                Xh(k) = X_1(k) + h;
                const DT f_plus = target_1(Xh);
                Xh(k) = X_1(k) - h;
                const DT f_minus = target_1(Xh);
                Xh(k) = X_1(k);
                grad(k) = (f_plus - f_minus) / (2 * h);
            }
            const DT f = target_1(X_1);
            if(f < best_f){ best_f = f; best_X = X_1; }  // the best point evaluated, kept if the line search gives up
            return f;
        };
        VectorD lower = VectorD::Constant(3*n, -std::numeric_limits<DT>::infinity());
        VectorD upper = VectorD::Constant(3*n, std::numeric_limits<DT>::infinity());
        VectorD init = VectorD::Zero(3*n);
        init = init.cwiseMax(lower).cwiseMin(upper);
        DT fx;
        LBFGSpp::LBFGSParam<DT> param;
        param.epsilon = 1e-8;
        param.max_iterations = 1000;
        LBFGSpp::LBFGSSolver<DT> solver(param);
        try
        {
            solver.minimize(helper, init, fx);
        }
        catch(const std::runtime_error &)
        {
            // LBFGSpp's line search could not decrease the objective further (e.g. near a
            // nonsmooth or exactly zero minimum): use the best point evaluated
            if(!(best_f < std::numeric_limits<DT>::infinity())) throw;
        }
        init = best_X;
        fx = best_f;
        ret = fx;
    }
};

