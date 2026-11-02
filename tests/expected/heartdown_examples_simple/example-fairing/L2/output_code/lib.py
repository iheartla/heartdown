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
        assert all( len(el) == 2 for el in E )
        assert np.ndim(n) == 0
        assert B.shape == (n,)
        assert V.shape == (n, 3)
        # w=10^6
        self.w = np.power(np.asarray(10, dtype=np.float64), 6)
        # A_ij = { 1 if (i,j) ∈ E
        #          1 if (j,i) ∈ E
        #          0 otherwise
        Aij_0 = []
        Avals_0 = []
        for tuple in E:
            i = tuple[0]+1
            j = tuple[1]+1
            Aij_0.append((i-1, j-1))
            Avals_0.append(1)
        for tuple_1 in E:
            j = tuple_1[0]+1
            i = tuple_1[1]+1
            Aij_0.append((i-1, j-1))
            Avals_0.append(1)
        sparse_0 = scipy.sparse.coo_matrix((Avals_0, np.asarray(Aij_0).T), shape=(n, n))
        self.A = sparse_0
        Dii_0 = []
        Dvals_0 = []
        for i in range(1, n+1):
            sum_0 = 0
            for j in range(1, self.A.shape[1]+1):
                sum_0 += self.A.tocsr()[i-1, j-1]
            Dii_0.append((i - 1, i - 1))
            Dvals_0.append(sum_0)
        self.D = scipy.sparse.coo_matrix((Dvals_0, np.asarray(Dii_0).T), shape=(n, n))
        # L = D⁻¹ ( D - A )
        self.L = sparse.linalg.spsolve(self.D, (self.D - self.A))
        # `V'` = (L² + w diag(B))⁻¹ (w diag(B) V)
        self.V_apostrophe = np.linalg.solve((sparse.linalg.matrix_power(self.L, 2) + self.w * np.diag(B)), (self.w * np.diag(B) @ V))

