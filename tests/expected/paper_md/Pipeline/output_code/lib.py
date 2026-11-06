import numpy as np
import scipy
import scipy.linalg
from scipy import sparse
from scipy.integrate import quad, solve_ivp
from scipy.optimize import minimize


class pipeline:
    def __init__(self, f_x, f_y, c_x, c_y, r, φ_circumflex_accent_1_d, φ_d, x_i, x_j, α_i, α_j, y, X, Y, Z, x):
        x_i = np.asarray(x_i, dtype=np.float64)
        x_j = np.asarray(x_j, dtype=np.float64)
        assert np.ndim(f_x) == 0
        assert np.ndim(f_y) == 0
        assert np.ndim(c_x) == 0
        assert np.ndim(c_y) == 0
        assert np.ndim(r) == 0
        assert x_i.shape == (2,)
        assert x_j.shape == (2,)
        assert np.ndim(α_i) == 0
        assert np.ndim(α_j) == 0
        assert np.ndim(y) == 0
        assert np.ndim(X) == 0
        assert np.ndim(Y) == 0
        assert np.ndim(Z) == 0
        assert np.ndim(x) == 0
        self.f_x = f_x
        self.f_y = f_y
        self.c_x = c_x
        self.c_y = c_y
        self.r = r
        self.φ_circumflex_accent_1_d = φ_circumflex_accent_1_d
        self.φ_d = φ_d
        self.x_i = x_i
        self.x_j = x_j
        self.α_i = α_i
        self.α_j = α_j
        self.y = y
        self.X = X
        self.Y = Y
        self.Z = Z
        self.x = x
        # `$\textbf{X}$` = (X,Y,Z)^T
        self.textbfX = (np.hstack((X, Y, Z,))).reshape(1, -1)
        # K = [`$f_x$` 0 `$c_x$`
        #       0   `$f_y$` `$c_y$`
        #       0      0    1]
        self.K = np.array([[f_x, 0, c_x], [0, f_y, c_y], [0, 0, 1]], dtype=np.float64)
        # `$\textbf{t}$` = [0;0;-r]
        self.textbft = np.array([[0], [0], [-r]], dtype=np.float64)
        # A = X ⋅ `$f_x$` - Z⋅(x - `$c_x$` )
        self.A = X * f_x - (Z * (x - c_x))
        # B = Z⋅`$f_x$` + X⋅(x -`$c_x$` )
        self.B = Z * f_x + X * (x - c_x)
        # D = √(A^2 +B^2)
        self.D = np.sqrt((np.power(self.A, 2) + np.power(self.B, 2)))
        # γ = arctan(B/A)
        self.γ = np.arctan(self.B / self.A)
        # C = -r⋅(x -`$c_x$` )
        self.C = -(r * (x - c_x))
        # ϕ = arcsin(C/D)
        self.ϕ = np.arcsin(self.C / self.D)
        # `$α_1$` = ϕ - γ
        self.α_1 = self.ϕ - self.γ
        # `$α_2$` = π - ϕ - γ
        self.α_2 = np.pi - self.ϕ - self.γ

    def R(self, α):
        return np.array([[-np.sin(α), 0, -np.cos(α)], [0, 1, 0], [np.cos(α), 0, -np.sin(α)]], dtype=np.float64)

    def P(self, α):
        return self.K @ np.block([[self.R(α), self.textbft]]).astype(np.float64)

    def t(self, α):
        return (α - self.α_i) / (self.α_j - self.α_i)

    def textbfx(self, α):
        return self.φ_circumflex_accent_1_d((1 - self.t(α)) * self.φ_d(self.x_i) + self.t(α) * self.φ_d(self.x_j))

    def ω(self, x):
        return np.arctan((x - self.c_x) / self.f_x)

    def s(self, x):
        return (self.y - self.c_y) * np.cos(self.ω(x))

    def φ(self, x):
        return np.hstack((self.ω(x), self.s(x),))

