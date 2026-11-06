import numpy as np
import scipy
import scipy.linalg
from scipy import sparse
from scipy.integrate import quad, solve_ivp
from scipy.optimize import minimize


class fairing:
    def __init__(self, E, n, B, V):
        E = frozenset(E)
        B = np.asarray(B, dtype=np.int64)
        V = np.asarray(V, dtype=np.float64)
        assert np.ndim(n) == 0
        assert B.shape == (n,)
        assert V.shape == (n, 3)
        self.E = E
        self.n = n
        self.B = B
        self.V = V
        # w=10^6
        self.w = 10**6
        # A_ij = { 1 if (i,j) ∈ E
        #          1 if (j,i) ∈ E
        #          0 otherwise
        rows_1 = []
        cols_2 = []
        vals_3 = []
        for i in range(1, n+1):
            for j in range(1, n+1):
                ret_4 = 0
                if (i-1, j-1) in E:
                    ret_4 = 1
                elif (j-1, i-1) in E:
                    ret_4 = 1
                else:
                    ret_4 = 0
                if ret_4 != 0:
                    rows_1.append(i-1)
                    cols_2.append(j-1)
                    vals_3.append(ret_4)
        self.A = sparse.csr_matrix((vals_3, (rows_1, cols_2)), shape=(n, n))
        # D_ii = ∑_j A_ij
        rows_5 = []
        cols_6 = []
        vals_7 = []
        for i in range(1, n+1):
            sum_8 = 0
            for j in range(1, n+1):
                sum_8 += self.A.tocsr()[i-1, j-1]
            if sum_8 != 0:
                rows_5.append(i-1)
                cols_6.append(i-1)
                vals_7.append(sum_8)
        self.D = sparse.csr_matrix((vals_7, (rows_5, cols_6)), shape=(n, n))
        # L = D⁻¹ ( D - A )
        self.L = sparse.linalg.spsolve(self.D, (self.D - self.A))
        # `V'` = (L² + w diag(B))⁻¹ (w diag(B) V)
        self.V_apostrophe = np.linalg.solve((sparse.linalg.matrix_power(self.L, 2) + self.w * np.diag(B)), ((self.w * np.diag(B)) @ V))

