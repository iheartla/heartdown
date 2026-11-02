#pragma once
#include <Eigen/Core>
#include <Eigen/QR>
#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <iostream>
#include <set>
#include <algorithm>
#include "type_helper.h"

struct computation {
    using DT = double;
    using MatrixD = Eigen::MatrixXd;
    using VectorD = Eigen::VectorXd;
    DT E;
    DT E_s;
    DT E_c;
    DT E_f;

    computation(
        const double & λ_s,
        const double & E_a,
        const std::function<double(double, double)> & D,
        const double & S_1,
        const double & S_2,
        const double & U_1,
        const double & U_2,
        const std::vector<Eigen::Matrix<double, 3, 1>> & R)
    {
        const long dim_0 = R.size();
        // `$E_c$` = D(`$S_1$`, `$U_1$`) + D(`$S_2$`, `$U_2$`)
        E_c = D(S_1, U_1) + D(S_2, U_2);
        // `$E_s$` = `$E_c$` + `$E_a$`
        E_s = E_c + E_a;
        // `$E_f$` = sum_i ||R_i||
        DT sum_0 = 0;
        for(int i=1; i<=R.size(); i++){
            sum_0 += (R.at(i-1)).template lpNorm<2>();
        }
        E_f = sum_0;
        // E = `$λ_s$``$E_s$` + (1 - `$λ_s$`)`$E_f$`
        E = λ_s * E_s + (1 - λ_s) * E_f;
    }
};

