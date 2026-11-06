import numpy as np
import scipy
import scipy.linalg
from scipy import sparse
from scipy.integrate import quad, solve_ivp
from scipy.optimize import minimize


class nautilus:
    def __init__(self, x, λ, v):
        x = np.asarray(x, dtype=np.float64)
        v = np.asarray(v, dtype=np.float64)
        n = x.shape[0]
        v_n = len(v)
        assert x.shape == (n,)
        assert np.ndim(λ) == 0
        self.x = x
        self.λ = λ
        self.v = v
        self.n = n
        self.v_n = v_n
        # P = [1 x₁ 0
        #      0 x₂ 0
        #      x₃ x₄ 1]
        self.P = np.array([[1, x[0], 0], [0, x[1], 0], [x[2], x[3], 1]], dtype=np.float64)
        # S = [x₅ x₆ x₇
        #      x₈ x₉ x₁₀
        #      0 0 1]
        self.S = np.array([[x[4], x[5], x[6]], [x[7], x[8], x[9]], [0, 0, 1]], dtype=np.float64)
        # H = PSP⁻¹
        self.H = (self.P @ self.S) @ (np.linalg.inv(self.P))

    def E(self, x):
        x = np.asarray(x, dtype=np.float64)
        sum_1 = 0
        for i in range(1, self.v_n+1):
            sum_1 += np.power(np.linalg.norm(self.H @ self.v[i-1] - ((self.P @ self.S) @ np.linalg.solve(self.P, self.v[i-1])), 'fro'), 2)
        return self.λ * (np.power(np.linalg.norm(np.hstack((x[4], x[7],)) - np.hstack((x[8], x[5],)), 2), 2)) + sum_1

