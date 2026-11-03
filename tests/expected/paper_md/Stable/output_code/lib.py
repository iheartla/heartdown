import numpy as np
import scipy
import scipy.linalg
from scipy import sparse
from scipy.integrate import quad, solve_ivp
from scipy.optimize import minimize


class Stable:
    def __init__(self, F, μ, I_C, λ, f_0, f_1, f_2):
        F = np.asarray(F, dtype=np.float64)
        f_0 = np.asarray(f_0, dtype=np.float64)
        f_1 = np.asarray(f_1, dtype=np.float64)
        f_2 = np.asarray(f_2, dtype=np.float64)
        assert F.shape == (3, 3)
        assert np.ndim(μ) == 0
        assert np.ndim(I_C) == 0
        assert np.ndim(λ) == 0
        assert f_0.shape == (3,)
        assert f_1.shape == (3,)
        assert f_2.shape == (3,)
        self.F = F
        self.μ = μ
        self.I_C = I_C
        self.λ = λ
        self.f_0 = f_0
        self.f_1 = f_1
        self.f_2 = f_2
        # α = 1 + μ/λ - μ/(4λ)
        self.α = 1 + μ / λ - (μ / (4 * λ))
        # J =`$f_0$`⋅(`$f_1$`×`$f_2$`)
        self.J = np.dot((f_0).ravel(), (np.cross(f_1, f_2)).ravel())
        # `$\frac{\partial J}{\partial F}$` = [`$f_1$`×`$f_2$` `$f_2$`×`$f_0$` `$f_0$`×`$f_1$`]
        self.fracpartialJpartialF = np.block([[(np.cross(f_1, f_2)).reshape(-1, 1), (np.cross(f_2, f_0)).reshape(-1, 1), (np.cross(f_0, f_1)).reshape(-1, 1)]]).astype(np.float64)
        # `$P(F)$` = μ(1-1/(`$I_C$`+1))F + λ(J- α)`$\frac{\partial J}{\partial F}$`
        self.PF = (μ * (1 - (1 / (I_C + 1)))) * F + (λ * (self.J - self.α)) * self.fracpartialJpartialF

