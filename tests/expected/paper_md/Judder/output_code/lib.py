import numpy as np
import scipy
import scipy.linalg
from scipy import sparse
from scipy.integrate import quad, solve_ivp
from scipy.optimize import minimize


class judder:
    def __init__(self, F, L, S, P, a, b, M, L_a, L_b):
        assert np.ndim(F) == 0
        assert np.ndim(L) == 0
        assert np.ndim(S) == 0
        assert np.ndim(a) == 0
        assert np.ndim(b) == 0
        assert np.ndim(M) == 0
        assert np.ndim(L_a) == 0
        assert np.ndim(L_b) == 0
        self.F = F
        self.L = L
        self.S = S
        self.P = P
        self.a = a
        self.b = b
        self.M = M
        self.L_a = L_a
        self.L_b = L_b
        # `$F_a$` = M⋅ CFF(`$L_a$`)
        self.F_a = M * self.CFF(L_a)
        # `$F_b$` = M⋅ CFF(`$L_b$`)
        self.F_b = M * self.CFF(L_b)
        # J = P(α(F), β(L), S)
        self.J = P(self.α(F), self.β(L), S)

    def CFF(self, L):
        return self.a * np.log(L) + self.b

    def α(self, F):
        return 1 / F

    def β(self, L):
        return np.log10(L)

class error:
    def __init__(self, O, M):
        O = np.asarray(O, dtype=np.float64)
        M = np.asarray(M, dtype=np.float64)
        N = O.shape[0]
        assert O.shape == (N,)
        assert M.shape == (N,)
        self.O = O
        self.M = M
        self.N = N
        # E = sum_i |log(O_i) - log(M_i)|/log(O_i)
        sum_1 = 0
        for i in range(1, N+1):
            sum_1 += np.absolute(np.log(O[i-1]) - np.log(M[i-1])) / np.log(O[i-1])
        self.E = sum_1

class third:
    def __init__(self, a, b, F_b, F_a, L_a):
        assert np.ndim(a) == 0
        assert np.ndim(b) == 0
        assert np.ndim(F_b) == 0
        assert np.ndim(F_a) == 0
        assert np.ndim(L_a) == 0
        self.a = a
        self.b = b
        self.F_b = F_b
        self.F_a = F_a
        self.L_a = L_a
        # `$L_b$` = 10^((a `$F_b$`log((`$L_a$`))+b(`$F_b$`-`$F_a$`))/(a`$F_a$`))
        self.L_b = 10**(((a * F_b) * np.log(L_a) + b * (F_b - F_a)) / (a * F_a))

