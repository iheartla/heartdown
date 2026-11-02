#pragma once
#include <Eigen/Core>
#include <Eigen/QR>
#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <iostream>
#include <set>
#include <algorithm>
#include "type_helper.h"

struct soft {
    using DT = double;
    using MatrixD = Eigen::MatrixXd;
    using VectorD = Eigen::VectorXd;
    DT F_S;

    soft(
        const std::function<double(int, Eigen::Matrix<double, 3, 1>)> & D,
        const std::vector<Eigen::Matrix<double, 3, 1>> & boldsymbolu,
        const double & σ,
        const std::vector<double> & α)
    {
        const long dim_0 = α.size();
        assert( boldsymbolu.size() == dim_0 );
        // `$F_S$` = sum_i α_i D_i(`$\boldsymbol{u}$`_i) + σ((sum_i α_i)/(sum_i α_i^2) - 1)
        DT sum_0 = 0;
        for(int i=1; i<=boldsymbolu.size(); i++){
            sum_0 += α.at(i-1) * D(i, (boldsymbolu.at(i-1)).eval());
        }
        DT sum_1 = 0;
        for(int i=1; i<=α.size(); i++){
            sum_1 += α.at(i-1);
        }
        DT sum_2 = 0;
        for(int i=1; i<=α.size(); i++){
            sum_2 += pow(α.at(i-1), 2);
        }
        F_S = sum_0 + σ * ((sum_1) / DT((sum_2)) - 1);
    }
};

