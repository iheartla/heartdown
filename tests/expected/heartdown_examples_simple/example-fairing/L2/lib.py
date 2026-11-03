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
        dense_1 = np.zeros((n, n))
        for i in range(1, n+1):
            for j in range(1, n+1):
                ret_2 = 0
                if (i-1, j-1) in E:
                    ret_2 = 1
                elif (j-1, i-1) in E:
                    ret_2 = 1
                else:
                    ret_2 = 0
                dense_1[i-1, j-1] = ret_2
        self.A = sparse.csr_matrix(dense_1)
        # D_ii = ∑_j A_ij
        dense_3 = np.zeros((n, n))
        for i in range(1, n+1):
            sum_4 = 0
            for j in range(1, n+1):
                sum_4 += self.A.tocsr()[i-1, j-1]
            dense_3[i-1, i-1] = sum_4
        self.D = sparse.csr_matrix(dense_3)
        # L = D⁻¹ ( D - A )
        self.L = sparse.linalg.spsolve(self.D, self.D - self.A)
        # `V'` = (L² + w diag(B))⁻¹ (w diag(B) V)
        self.V_apostrophe = np.linalg.solve(sparse.linalg.matrix_power(self.L, 2) + self.w * np.diag(B), (self.w * np.diag(B)) @ V)

