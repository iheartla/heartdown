#pragma once
#include <Eigen/Core>
#include <Eigen/QR>
#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <iostream>
#include <set>
#include <algorithm>
#include "type_helper.h"

struct filters {
    using DT = double;
    using MatrixD = Eigen::MatrixXd;
    using VectorD = Eigen::VectorXd;


    template<typename REAL>
    REAL f(
        const REAL & x)
    {
        REAL f_ret;
        if(abs(x) < 1){
            f_ret = 1 - abs(x);
        }
        else{
            f_ret = 0;
        }
        return f_ret;    
    }
    filters()
    {
    
    
    }
};

