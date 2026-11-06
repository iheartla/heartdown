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

struct filters {
    using DT = double;

    filters()
    {

    }

    double f(const double & x)
    {
        return [&](){ if(fabs(x) < (1/double(3))){ return double(1 - 3 * pow(x, 2)); } else if(fabs(x) < 1){ return double(1.5 * pow(x, 2) - 3 * fabs(x) + 1.5); } return double(0); }();
    }
};

