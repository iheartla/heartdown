import numpy as np
import scipy
import scipy.linalg
from scipy import sparse
from scipy.integrate import quad, solve_ivp
from scipy.optimize import minimize


class filters:
    def __init__(self, ):
        pass

    def f(self, x):
        assert np.ndim(x) == 0

        if np.absolute(x) < 1:
            f_ret = 1 - np.absolute(x)
        else:
            f_ret = 0
        return f_ret

