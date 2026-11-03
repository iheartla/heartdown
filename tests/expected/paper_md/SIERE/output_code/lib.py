import numpy as np
import scipy
import scipy.linalg
from scipy import sparse
from scipy.integrate import quad, solve_ivp
from scipy.optimize import minimize


class siere:
    def __init__(self, U_s, M, v, f, K, boldsymbolI, h, φ_1, u):
        U_s = np.asarray(U_s, dtype=np.float64)
        M = np.asarray(M, dtype=np.float64)
        v = np.asarray(v, dtype=np.float64)
        f = np.asarray(f, dtype=np.float64)
        K = np.asarray(K, dtype=np.float64)
        boldsymbolI = np.asarray(boldsymbolI, dtype=np.float64)
        u = np.asarray(u, dtype=np.float64)
        n = U_s.shape[0]
        s = U_s.shape[1]
        assert U_s.shape == (n, s)
        assert M.shape == (n, n)
        assert v.shape == (n,)
        assert f.shape == (n,)
        assert K.shape == (n, n)
        assert boldsymbolI.shape == (2*n, 2*n)
        assert np.ndim(h) == 0
        assert u.shape == (2*n, 1)
        self.U_s = U_s
        self.M = M
        self.v = v
        self.f = f
        self.K = K
        self.boldsymbolI = boldsymbolI
        self.h = h
        self.φ_1 = φ_1
        self.u = u
        self.n = n
        self.s = s
        # `$v_G$` = `$U_s$``$U_s$`^T Mv
        self.v_G = ((U_s @ U_s.T) @ M) @ v
        # `$v_H$` =  v - `$v_G$`
        self.v_H = v - self.v_G
        # `$f_G$` =  M`$U_s$``$U_s$`^T f
        self.f_G = ((M @ U_s) @ U_s.T) @ f
        # `$G(u)$` = [`$v_G$`
        #           M⁻¹`$f_G$`]
        self.Gu = np.block([[(self.v_G).reshape(-1, 1)], [(np.linalg.solve(M, self.f_G)).reshape(-1, 1)]]).astype(np.float64)
        # `$f_H$` =  f - `$f_G$`
        self.f_H = f - self.f_G
        # `$H(u)$` = [`$v_H$`
        #           M⁻¹`$f_H$`]
        self.Hu = np.block([[(self.v_H).reshape(-1, 1)], [(np.linalg.solve(M, self.f_H)).reshape(-1, 1)]]).astype(np.float64)
        # `$J_G$` = [0    `$U_s$``$U_s$`^TM
        #       -`$U_s$``$U_s$`^TK`$U_s$``$U_s$`^TM 0 ]
        self.J_G = np.block([[np.zeros((n, n)), (U_s @ U_s.T) @ M], [-((((U_s @ U_s.T) @ K) @ U_s) @ U_s.T) @ M, np.zeros((n, n))]]).astype(np.float64)
        # `$J_H$` =  [0     I_n
        #             -M⁻¹K 0] - `$J_G$`
        self.J_H = np.block([[np.zeros((n, n)), np.identity(n)], [-np.linalg.solve(M, K), np.zeros((n, n))]]).astype(np.float64) - self.J_G
        # `$J_G^r$` = [0     I_s
        #             -`$U_s$`^TK`$U_s$` 0]
        self.J_G_circumflex_accent_r = np.block([[np.zeros((s, s)), np.identity(s)], [-(U_s.T @ K) @ U_s, np.zeros((s, s))]]).astype(np.float64)
        # `$G^r(u)$` = [`$U_s$`^TMv
        #             `$U_s$`^Tf]
        self.G_circumflex_accent_ru = np.block([[((U_s.T @ M) @ v).reshape(-1, 1)], [(U_s.T @ f).reshape(-1, 1)]]).astype(np.float64)
        # `$u_+$` =  u + (`$\boldsymbol{I}$` -h`$J_H$`)⁻¹(h `$H(u)$` + h[`$U_s$` 0
        #                                                0   `$U_s$`] `$φ_1$`(h`$J_G^r$`) `$G^r(u)$`)
        self.u_plus_sign = u + np.linalg.solve(boldsymbolI - (h * self.J_H), h * self.Hu + ((h * np.block([[U_s, np.zeros((n, s))], [np.zeros((n, s)), U_s]]).astype(np.float64)) @ φ_1(h * self.J_G_circumflex_accent_r)) @ self.G_circumflex_accent_ru)

class second:
    def __init__(self, U_s, M, K):
        U_s = np.asarray(U_s, dtype=np.float64)
        M = np.asarray(M, dtype=np.float64)
        K = np.asarray(K, dtype=np.float64)
        n = U_s.shape[0]
        s = U_s.shape[1]
        assert U_s.shape == (n, s)
        assert M.shape == (n, n)
        assert K.shape == (n, n)
        self.U_s = U_s
        self.M = M
        self.K = K
        self.n = n
        self.s = s
        # `$Y_1$` =  [`$U_s$`
        #              0]
        self.Y_1 = np.block([[U_s], [np.zeros((1, s))]]).astype(np.float64)
        # `$Z_1$` =  [ 0
        #              M`$U_s$`]
        self.Z_1 = np.block([[np.zeros((1, s))], [M @ U_s]]).astype(np.float64)
        # `$Y_2$` =  [0
        #             -`$U_s$``$U_s$`^TK`$U_s$`]
        self.Y_2 = np.block([[np.zeros((1, s))], [-((U_s @ U_s.T) @ K) @ U_s]]).astype(np.float64)
        # `$Z_2$` =  [ M`$U_s$`
        #              0]
        self.Z_2 = np.block([[M @ U_s], [np.zeros((1, s))]]).astype(np.float64)
        # `$J_G$` = `$Y_1$``$Z_1$`^T + `$Y_2$``$Z_2$`^T
        self.J_G = self.Y_1 @ self.Z_1.T + self.Y_2 @ self.Z_2.T

