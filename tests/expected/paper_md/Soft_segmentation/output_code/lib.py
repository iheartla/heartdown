import numpy as np
import scipy
import scipy.linalg
from scipy import sparse
from scipy.integrate import quad, solve_ivp
from scipy.optimize import minimize


class soft:
    def __init__(self, D, boldsymbolu, σ, α):
        boldsymbolu = np.asarray(boldsymbolu, dtype=np.float64)
        α = np.asarray(α, dtype=np.float64)
        boldsymbolu_n = len(boldsymbolu)
        α_n = len(α)
        assert np.ndim(σ) == 0
        self.D = D
        self.boldsymbolu = boldsymbolu
        self.σ = σ
        self.α = α
        self.boldsymbolu_n = boldsymbolu_n
        self.α_n = α_n
        # `$F_S$` = sum_i α_i D_i(`$\boldsymbol{u}$`_i) + σ((sum_i α_i)/(sum_i α_i^2) - 1)
        sum_1 = 0
        for i in range(1, α_n+1):
            sum_1 += α[i-1] * D(i, boldsymbolu[i-1])
        sum_2 = 0
        for i in range(1, α_n+1):
            sum_2 += α[i-1]
        sum_3 = 0
        for i in range(1, α_n+1):
            sum_3 += α[i-1]**2
        self.F_S = sum_1 + σ * (sum_2 / sum_3 - 1)

