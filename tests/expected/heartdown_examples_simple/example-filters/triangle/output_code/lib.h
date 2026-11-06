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
        return [&](){ if(fabs(x) < 1){ return double(1 - fabs(x)); } return double(0); }();
    }
};

