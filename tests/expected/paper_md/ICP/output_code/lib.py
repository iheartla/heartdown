import numpy as np
import scipy
import scipy.linalg
from scipy import sparse
from scipy.integrate import quad, solve_ivp
from scipy.optimize import minimize


class ICP:
    def __init__(self, R, a, θ, p, q, n_q, n_p, t, barq, barp, trans, rot):
        R = np.asarray(R, dtype=np.float64)
        a = np.asarray(a, dtype=np.float64)
        p = np.asarray(p, dtype=np.float64)
        q = np.asarray(q, dtype=np.float64)
        n_q = np.asarray(n_q, dtype=np.float64)
        n_p = np.asarray(n_p, dtype=np.float64)
        t = np.asarray(t, dtype=np.float64)
        barq = np.asarray(barq, dtype=np.float64)
        barp = np.asarray(barp, dtype=np.float64)
        p_n = len(p)
        q_n = len(q)
        n_q_n = len(n_q)
        n_p_n = len(n_p)
        assert R.shape == (3, 3)
        assert a.shape == (3,)
        assert np.ndim(θ) == 0
        assert t.shape == (3,)
        assert barq.shape == (3,)
        assert barp.shape == (3,)
        self.R = R
        self.a = a
        self.θ = θ
        self.p = p
        self.q = q
        self.n_q = n_q
        self.n_p = n_p
        self.t = t
        self.barq = barq
        self.barp = barp
        self.trans = trans
        self.rot = rot
        self.p_n = p_n
        self.q_n = q_n
        self.n_q_n = n_q_n
        self.n_p_n = n_p_n
        # ã = a tan(θ)
        self.ã = a * np.tan(θ)
        # n_i = `$n_q$`_i + `$n_p$`_i
        seq_1 = []
        for i in range(1, n_q_n+1):
            seq_1.append(n_q[i-1] + n_p[i-1])
        self.n = np.asarray(seq_1)
        # t̃ = t/cos(θ)
        self.t̃ = t / np.cos(θ)
        # p̃_i = p_i - `$\bar{p}$`
        seq_2 = []
        for i in range(1, p_n+1):
            seq_2.append(p[i-1] - barp)
        self.p̃ = np.asarray(seq_2)
        # q̃_i = q_i - `$\bar{q}$`
        seq_3 = []
        for i in range(1, q_n+1):
            seq_3.append(q[i-1] - barq)
        self.q̃ = np.asarray(seq_3)
        # `$\varepsilon_{point}$` = ∑_i ||R p_i + t - q_i||
        sum_4 = 0
        for i in range(1, p_n+1):
            sum_4 += np.linalg.norm(R @ p[i-1] + t - q[i-1], 2)
        self.varepsilon_point = sum_4
        # `$\varepsilon_{plane}$` = ∑_i ((R p_i + t - q_i) ⋅ `$n_q$`_i)^2
        sum_5 = 0
        for i in range(1, p_n+1):
            sum_5 += np.dot((R @ p[i-1] + t - q[i-1]).ravel(), (n_q[i-1]).ravel())**2
        self.varepsilon_plane = sum_5
        # `$\varepsilon_{symm-RN}$` = ∑_i ((R p_i + R⁻¹ q_i + t) ⋅ (R`$n_p$`_i + R⁻¹`$n_q$`_i))^2
        sum_6 = 0
        for i in range(1, p_n+1):
            sum_6 += np.dot((R @ p[i-1] + np.linalg.solve(R, q[i-1]) + t).ravel(), (R @ n_p[i-1] + np.linalg.solve(R, n_q[i-1])).ravel())**2
        self.varepsilon_symmRN = sum_6
        # `$\varepsilon_{symm}$` = ∑_i cos²(θ)((p_i - q_i)⋅n_i +((p_i+q_i)×n_i)⋅ã+n_i⋅t̃)²
        sum_7 = 0
        for i in range(1, p_n+1):
            sum_7 += (np.cos(θ)**2) * ((np.dot((p[i-1] - q[i-1]).ravel(), (self.n[i-1]).ravel()) + np.dot((np.cross(p[i-1] + q[i-1], self.n[i-1])).ravel(), (self.ã).ravel()) + np.dot((self.n[i-1]).ravel(), (self.t̃).ravel()))**2)
        self.varepsilon_symm = sum_7
        # S = trans(`$\bar{q}$`) ⋅ rot(θ, ã/||ã||) ⋅trans(t̃ cos(θ)) ⋅rot(θ, ã/||ã||)⋅ trans(-`$\bar{p}$`)
        self.S = (((trans(barq) @ rot(θ, self.ã / np.linalg.norm(self.ã, 2))) @ trans(self.t̃ * np.cos(θ))) @ rot(θ, self.ã / np.linalg.norm(self.ã, 2))) @ trans(-barp)
        # `$\varepsilon_{two-plane}$` = ∑_i(((R p_i + R⁻¹ q_i + t) ⋅ (R `$n_p$`_i))^2 + ((R p_i + R⁻¹ q_i + t) ⋅ (R⁻¹`$n_q$`_i))^2)
        sum_8 = 0
        for i in range(1, p_n+1):
            sum_8 += np.dot((R @ p[i-1] + np.linalg.solve(R, q[i-1]) + t).ravel(), (R @ n_p[i-1]).ravel())**2 + np.dot((R @ p[i-1] + np.linalg.solve(R, q[i-1]) + t).ravel(), (np.linalg.solve(R, n_q[i-1])).ravel())**2
        self.varepsilon_twoplane = sum_8

