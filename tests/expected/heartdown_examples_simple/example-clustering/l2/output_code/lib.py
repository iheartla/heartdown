import numpy as np
import scipy
import scipy.linalg
from scipy import sparse
from scipy.integrate import quad, solve_ivp
from scipy.optimize import minimize


class clustering:
    def __init__(self, y):
        y = np.asarray(y, dtype=np.float64)
        dim_0 = y.shape[0]
        m = y.shape[1]
        assert y.shape == (dim_0, m, )

        self.y = y
        def pack(c):
            return np.concatenate([c])
        def unpack(X):
            return X[0:m]
        def target(X):
            c = unpack(X)
            sum_0 = 0
            for i in range(1, len(y)+1):
                sum_0 += np.power(np.asarray(np.linalg.norm(y[i-1] - c, 2), dtype=np.float64), 2)
            return sum_0
        opt = minimize(target, np.zeros(m))
        c = unpack(opt.x)
        self.ret = opt.fun
        self.c = c

