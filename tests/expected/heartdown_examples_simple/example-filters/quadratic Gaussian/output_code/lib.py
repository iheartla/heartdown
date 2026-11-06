import numpy as np
import scipy
import scipy.linalg
from scipy import sparse
from scipy.integrate import quad, solve_ivp
from scipy.optimize import minimize


class filters:
    def __init__(self):
        pass

    def f(self, x):
        ret_1 = 0
        if np.absolute(x) < (1/3):
            ret_1 = 1 - (3 * (np.power(x, 2)))
        elif np.absolute(x) < 1:
            ret_1 = (1.5) * (np.power(x, 2)) - (3 * np.absolute(x)) + 1.5
        else:
            ret_1 = 0
        return ret_1

