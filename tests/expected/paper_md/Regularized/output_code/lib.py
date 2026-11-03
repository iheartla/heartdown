import numpy as np
import scipy
import scipy.linalg
from scipy import sparse
from scipy.integrate import quad, solve_ivp
from scipy.optimize import minimize


class Regularized:
    def __init__(self, r, boldsymbolf, s, boldsymbolF, r_ε, a, b, ε):
        boldsymbolf = np.asarray(boldsymbolf, dtype=np.float64)
        boldsymbolF = np.asarray(boldsymbolF, dtype=np.float64)
        assert np.ndim(r) == 0
        assert boldsymbolf.shape == (3,)
        assert np.ndim(s) == 0
        assert boldsymbolF.shape == (3, 3)
        assert np.ndim(r_ε) == 0
        assert np.ndim(a) == 0
        assert np.ndim(b) == 0
        assert np.ndim(ε) == 0
        self.r = r
        self.boldsymbolf = boldsymbolf
        self.s = s
        self.boldsymbolF = boldsymbolF
        self.r_ε = r_ε
        self.a = a
        self.b = b
        self.ε = ε

    def boldsymbolu(self, boldsymbolr):
        boldsymbolr = np.asarray(boldsymbolr, dtype=np.float64)
        return (((self.a - self.b) / self.r) * np.identity(3) + (((self.b / (self.r**3)) * boldsymbolr)).reshape(-1, 1) @ (boldsymbolr).reshape(1, -1)) @ self.boldsymbolf

    def rho_ε(self, boldsymbolr):
        boldsymbolr = np.asarray(boldsymbolr, dtype=np.float64)
        return (15 * (self.r_ε**4)) / (8 * np.pi) + 1 / (self.r_ε**7)

    def boldsymbolu_ε(self, boldsymbolr):
        boldsymbolr = np.asarray(boldsymbolr, dtype=np.float64)
        return (((self.a - self.b) / self.r_ε) * np.identity(3) + (((self.b / (self.r_ε**3)) * boldsymbolr)).reshape(-1, 1) @ (boldsymbolr).reshape(1, -1) + ((self.a * (self.ε**2)) / (2 * (self.r_ε**3))) * np.identity(3)) @ self.boldsymbolf

    def tildeboldsymbolu_ε(self, boldsymbolr):
        boldsymbolr = np.asarray(boldsymbolr, dtype=np.float64)
        return -((self.a * (1 / (self.r_ε**3) + (3 * (self.ε**2)) / (2 * (self.r_ε**5)))) * self.boldsymbolF) @ boldsymbolr + (self.b * ((1 / (self.r_ε**3)) * (self.boldsymbolF + self.boldsymbolF.T + np.trace(self.boldsymbolF) * np.identity(3)) - (((3 / (self.r_ε**5)) * (np.asarray(((boldsymbolr).reshape(1, -1) @ self.boldsymbolF) @ boldsymbolr).item())) * np.identity(3)))) @ boldsymbolr

    def boldsymbolt_ε(self, boldsymbolr):
        boldsymbolr = np.asarray(boldsymbolr, dtype=np.float64)
        return -((self.a * (1 / (self.r_ε**3) + (3 * (self.ε**2)) / (2 * (self.r_ε**5)))) * self.boldsymbolF) @ boldsymbolr

    def boldsymbols_ε(self, boldsymbolr):
        boldsymbolr = np.asarray(boldsymbolr, dtype=np.float64)
        return ((2 * self.b - self.a) * (1 / (self.r_ε**3) + (3 * (self.ε**2)) / (2 * (self.r_ε**5)))) * (self.s * boldsymbolr)

    def boldsymbolp_ε(self, boldsymbolr):
        boldsymbolr = np.asarray(boldsymbolr, dtype=np.float64)
        return (((2 * self.b - self.a) / (self.r_ε**3)) * self.boldsymbolF) @ boldsymbolr - (((3 / (2 * (self.r_ε**5))) * (((2 * self.b) * (np.asarray(((boldsymbolr).reshape(1, -1) @ self.boldsymbolF) @ boldsymbolr).item())) * np.identity(3) + (self.a * (self.ε**2)) * self.boldsymbolF)) @ boldsymbolr)

