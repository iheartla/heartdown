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

struct clustering {
    using DT = double;
    using MatrixD = Eigen::MatrixXd;
    using VectorD = Eigen::VectorXd;
    DT ret;
    std::vector<Eigen::VectorXd> y;
    clustering(const std::vector<Eigen::VectorXd> & y)
    {
        const long dim_0 = y.size();
        const long m = y[0].rows();
        assert( y.size() == dim_0 );
        for( const auto& el : y ) {
            assert( el.size() == m );
        }
        this->y = y;
        auto unpack = [&](const VectorD & X_1, VectorD & c)
        {
            c = X_1.segment(0, m);
        };
        auto target_1 = [&](const VectorD & X_1)
        {
            VectorD c;
            unpack(X_1, c);
            DT sum_0 = 0;
            for(int i=1; i<=y.size(); i++){
                sum_0 += pow((y.at(i-1) - c).template lpNorm<2>(), 1);
            }
            return DT(sum_0);
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
        VectorD lower = VectorD::Constant(m, -std::numeric_limits<DT>::infinity());
        VectorD upper = VectorD::Constant(m, std::numeric_limits<DT>::infinity());
        VectorD init = VectorD::Zero(m);
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

