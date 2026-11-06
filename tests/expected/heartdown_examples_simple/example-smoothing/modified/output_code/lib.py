import numpy as np
import scipy
import scipy.linalg
from scipy import sparse
from scipy.integrate import quad, solve_ivp
from scipy.optimize import minimize


class MeshSmoothing:
    def __init__(self, V, L, λ):
        V = np.asarray(V, dtype=np.float64)
        n = V.shape[0]
        assert V.shape == (n, 3)
        assert np.ndim(λ) == 0
        self.V = V
        self.L = L
        self.λ = λ
        self.n = n
        # min_( U ∈ ℝ^(n×3) ) (1-λ) `E_\text{data}`(U) + λ `E_\text{smoothness}`(U)
        def objective_2(x_1):
            U = x_1[0:0+(n)*(3)].reshape((n, 3))
            return ((1 - λ) * self.E_textdata(U) + λ * self.E_textsmoothness(U))
        res_3 = minimize(objective_2, np.zeros((n)*(3)))
        self.U = res_3.x[0:0+(n)*(3)].reshape((n, 3))
        self.ret = res_3.fun

    def E_textdata(self, U):
        U = np.asarray(U, dtype=np.float64)
        return np.power(np.linalg.norm(U - self.V, 'fro'), 2)

    def E_textsmoothness(self, U):
        U = np.asarray(U, dtype=np.float64)
        return np.trace((U.T @ self.L) @ U)

