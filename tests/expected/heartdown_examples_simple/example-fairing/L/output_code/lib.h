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
#ifndef IHEARTLA_SORTED_SET
#define IHEARTLA_SORTED_SET
// sets are sorted vectors without duplicates (defined before the last #include:
// code that wraps the generated code in a namespace starts it after that)
template<typename T> std::vector<T> sorted_set(std::vector<T> v) { std::sort(v.begin(), v.end()); v.erase(std::unique(v.begin(), v.end()), v.end()); return v; }
// (vectors in lexicographic order)
template<typename S, int R, int C, int O, int MR, int MC> std::vector<Eigen::Matrix<S, R, C, O, MR, MC>> sorted_set(std::vector<Eigen::Matrix<S, R, C, O, MR, MC>> v) {
using M = Eigen::Matrix<S, R, C, O, MR, MC>;
std::sort(v.begin(), v.end(), [](const M & a, const M & b){ return std::lexicographical_compare(a.data(), a.data() + a.size(), b.data(), b.data() + b.size()); });
v.erase(std::unique(v.begin(), v.end(), [](const M & a, const M & b){ return a.size() == b.size() && (a.array() == b.array()).all(); }), v.end());
return v;
}
#endif
#include <unsupported/Eigen/MatrixFunctions>

struct fairing {
    using DT = double;
    std::vector<std::tuple<int, int>> E;
    int n;
    Eigen::Matrix<int, Eigen::Dynamic, 1> B;
    Eigen::MatrixXd V;
    int w;
    Eigen::SparseMatrix<double> A;
    Eigen::SparseMatrix<double> D;
    Eigen::SparseMatrix<double> L;
    Eigen::MatrixXd V_apostrophe;

    fairing(const std::vector<std::tuple<int, int>> & E, const int & n, const Eigen::Matrix<int, Eigen::Dynamic, 1> & B, const Eigen::MatrixXd & V)
    {
        this->E = E;
        this->n = n;
        this->B = B;
        this->V = V;
        assert( B.rows() == this->n );
        assert( V.rows() == this->n && V.cols() == 3 );
        // w=10^6
        this->w = pow(10, 6);
        // A_ij = { 1 if (i,j) ∈ E
        //          1 if (j,i) ∈ E
        //          0 otherwise
        std::vector<Eigen::Triplet<double> > triplets_1;
        for(int i = 1; i <= this->n; i++){
            for(int j = 1; j <= this->n; j++){
                double value_2 = [&](){ if([&](){ const auto & s = this->E; return std::find(s.begin(), s.end(), std::make_tuple(i-1, j-1)) != s.end(); }()){ return int(1); } else if([&](){ const auto & s = this->E; return std::find(s.begin(), s.end(), std::make_tuple(j-1, i-1)) != s.end(); }()){ return int(1); } return int(0); }();
                if(value_2 != 0) triplets_1.push_back(Eigen::Triplet<double>(i-1, j-1, value_2));
            }
        }
        Eigen::SparseMatrix<double> entries_3(this->n, this->n);
        entries_3.setFromTriplets(triplets_1.begin(), triplets_1.end());
        this->A = entries_3;
        // D_ii = ∑_j A_ij
        std::vector<Eigen::Triplet<double> > triplets_4;
        for(int i = 1; i <= this->n; i++){
            double value_5 = [&](){ double sum = 0.0; for(int j = 1; j <= this->n; j++){ sum += this->A.coeff(i-1, j-1); } return sum; }();
            if(value_5 != 0) triplets_4.push_back(Eigen::Triplet<double>(i-1, i-1, value_5));
        }
        Eigen::SparseMatrix<double> entries_6(this->n, this->n);
        entries_6.setFromTriplets(triplets_4.begin(), triplets_4.end());
        this->D = entries_6;
        // L = D⁻¹ ( D - A )
        this->L = (Eigen::MatrixXd(this->D)).colPivHouseholderQr().solve(Eigen::MatrixXd((this->D - this->A)));
        // `V'` = (L + w diag(B))⁻¹ (w diag(B) V)
        this->V_apostrophe = (((this->L + this->w * Eigen::MatrixXd((this->B).asDiagonal()))).eval()).colPivHouseholderQr().solve((((this->w * Eigen::MatrixXd((this->B).asDiagonal())) * this->V)).eval());
    }
};

