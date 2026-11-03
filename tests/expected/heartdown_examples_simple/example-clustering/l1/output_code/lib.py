import numpy as np
import scipy
import scipy.linalg
from scipy import sparse
from scipy.integrate import quad, solve_ivp
from scipy.optimize import minimize


class clustering:
    def __init__(self, y):
        y = np.asarray(y, dtype=np.float64)
        y_n = len(y)
        m = np.asarray(y[0]).shape[0]
        self.y = y
        self.y_n = y_n
        self.m = m
        # min_( c ∈ ℝ^m ) ∑_i ‖ y_i - c ‖^1
        def objective_2(x_1):
            c = x_1[0:0+m]
            sum_3 = 0
            for i in range(1, y_n+1):
                sum_3 += np.linalg.norm(y[i-1] - c, 2)**1
            return (sum_3)
        res_4 = minimize(objective_2, np.zeros(m))
        self.c = res_4.x[0:0+m]
        self.ret = res_4.fun

