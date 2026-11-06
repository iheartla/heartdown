import numpy as np
import scipy
import scipy.linalg
from scipy import sparse
from scipy.integrate import quad, solve_ivp
from scipy.optimize import minimize


class clustering:
    def __init__(self, y, w):
        y = np.asarray(y, dtype=np.float64)
        w = np.asarray(w, dtype=np.float64)
        y_n = len(y)
        m = np.asarray(y[0]).shape[0]
        w_n = len(w)
        self.y = y
        self.w = w
        self.y_n = y_n
        self.m = m
        self.w_n = w_n
        # min_( c ∈ ℝ^m ) ∑_i w_i ‖ y_i - c ‖^2
        def objective_2(x_1):
            c = x_1[0:0+m]
            sum_3 = 0
            for i in range(1, w_n+1):
                sum_3 += w[i-1] * (np.power(np.linalg.norm(y[i-1] - c, 2), 2))
            return (sum_3)
        res_4 = minimize(objective_2, np.zeros(m))
        self.c = res_4.x[0:0+m]
        self.ret = res_4.fun

