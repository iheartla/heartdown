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
        assert L.shape == (n, n)
        assert np.ndim(λ) == 0

        self.V = V
        self.L = L
        self.λ = λ
        def pack(U):
            return np.concatenate([np.reshape(U, -1)])
        def unpack(X):
            return X[0:3*n].reshape(n, 3)
        def target(X):
            U = unpack(X)
            return self.E_textdata(U) + λ * self.E_textsmoothness(U)
        opt = minimize(target, np.zeros(3*n))
        U = unpack(opt.x)
        self.ret = opt.fun
        self.U = U

    def E_textdata(self, U):
        U = np.asarray(U, dtype=np.float64)
        n = U.shape[0]
        assert U.shape == (n, 3)

        return np.power(np.asarray(np.linalg.norm(U - self.V, 'fro'), dtype=np.float64), 2)

    def E_textsmoothness(self, U):
        U = np.asarray(U, dtype=np.float64)
        n = U.shape[0]
        assert U.shape == (n, 3)

        return np.trace(U.T @ self.L @ U)

