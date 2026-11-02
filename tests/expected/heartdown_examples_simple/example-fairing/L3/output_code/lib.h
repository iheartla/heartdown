#pragma once
#include <Eigen/Core>
#include <Eigen/QR>
#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <iostream>
#include <set>
#include <algorithm>
#include <unsupported/Eigen/MatrixFunctions>
#include "type_helper.h"

struct fairing {
    using DT = double;
    using MatrixD = Eigen::MatrixXd;
    using VectorD = Eigen::VectorXd;
    DT w;
    Eigen::SparseMatrix<double> A;
    Eigen::SparseMatrix<double> D;
    Eigen::SparseMatrix<double> L;
    Eigen::Matrix<DT, Eigen::Dynamic, 3> V_apostrophe;

    fairing(
        const std::vector<std::tuple< int, int > > & E,
        const int & n,
        const Eigen::VectorXi & B,
        const Eigen::Matrix<double, Eigen::Dynamic, 3> & V)
    {
        assert( B.size() == n );
        assert( V.rows() == n );
        assert( V.cols() == 3 );
        // w=10^6
        w = pow(10, 6);
        // A_ij = { 1 if (i,j) ∈ E
        //          1 if (j,i) ∈ E
        //          0 otherwise
        A.resize(n, n);
        std::vector<Eigen::Triplet<DT> > tripletList_A;
        for(const std::tuple< int, int >& tuple : E){
            int i = std::get<0>(tuple)+1;
            int j = std::get<1>(tuple)+1;
            tripletList_A.push_back(Eigen::Triplet<DT>(i-1, j-1, 1));
        }
        for(const std::tuple< int, int >& tuple : E){
            int j = std::get<0>(tuple)+1;
            int i = std::get<1>(tuple)+1;
            tripletList_A.push_back(Eigen::Triplet<DT>(i-1, j-1, 1));
        }
        A.setFromTriplets(tripletList_A.begin(), tripletList_A.end());
        // D_ii = ∑_j A_ij
        D.resize(n, n);
        std::vector<Eigen::Triplet<DT> > tripletList_D;
        for( int i=1; i<=n; i++){
            DT sum_0 = 0;
            for(int j=1; j<=A.cols(); j++){
                sum_0 += this->A.coeff(i-1, j-1);
            }
            tripletList_D.push_back(Eigen::Triplet<DT>(i-1, i-1, sum_0));
        }
        D.setFromTriplets(tripletList_D.begin(), tripletList_D.end());
        // L = D⁻¹ ( D - A )
        Eigen::SimplicialLLT <Eigen::SparseMatrix<double>> solver;
        solver.compute(D);
        L = solver.solve((D - A));
        // `V'` = (L³ + w diag(B))⁻¹ (w diag(B) V)
        Eigen::MatrixPower<Eigen::SparseMatrix<double>> pow(L);
        V_apostrophe = (to_double((pow(3) + w * (B).asDiagonal()))).colPivHouseholderQr().solve(to_double((w * (B).asDiagonal() * V)));
    }
};

