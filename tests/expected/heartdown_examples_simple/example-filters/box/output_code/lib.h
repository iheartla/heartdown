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

    int f(const double & x)
    {
        return [&](){ if(fabs(x) < 1){ return int(1); } return int(0); }();
    }
};

