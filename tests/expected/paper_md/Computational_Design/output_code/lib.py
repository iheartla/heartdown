import numpy as np
import scipy
import scipy.linalg
from scipy import sparse
from scipy.integrate import quad, solve_ivp
from scipy.optimize import minimize


class computation:
    def __init__(self, λ_s, E_a, D, S_1, S_2, U_1, U_2, R):
        R = np.asarray(R, dtype=np.float64)
        R_n = len(R)
        assert np.ndim(λ_s) == 0
        assert np.ndim(E_a) == 0
        assert np.ndim(S_1) == 0
        assert np.ndim(S_2) == 0
        assert np.ndim(U_1) == 0
        assert np.ndim(U_2) == 0
        self.λ_s = λ_s
        self.E_a = E_a
        self.D = D
        self.S_1 = S_1
        self.S_2 = S_2
        self.U_1 = U_1
        self.U_2 = U_2
        self.R = R
        self.R_n = R_n
        # `$E_c$` = D(`$S_1$`, `$U_1$`) + D(`$S_2$`, `$U_2$`)
        self.E_c = D(S_1, U_1) + D(S_2, U_2)
        # `$E_s$` = `$E_c$` + `$E_a$`
        self.E_s = self.E_c + E_a
        # `$E_f$` = sum_i ||R_i||
        sum_1 = 0
        for i in range(1, R_n+1):
            sum_1 += np.linalg.norm(R[i-1], 2)
        self.E_f = sum_1
        # E = `$λ_s$``$E_s$` + (1 - `$λ_s$`)`$E_f$`
        self.E = λ_s * self.E_s + (1 - λ_s) * self.E_f

