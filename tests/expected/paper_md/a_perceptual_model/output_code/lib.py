import numpy as np
import scipy
import scipy.linalg
from scipy import sparse
from scipy.integrate import quad, solve_ivp
from scipy.optimize import minimize


class perceptual:
    def __init__(self, CSF, ω, m_combining_tilde_t_comma_b, β_b, σ_P_circumflex_accent_A, σ_P_circumflex_accent_B, σ_O_circumflex_accent_A, σ_O_circumflex_accent_B):
        ω = np.asarray(ω, dtype=np.float64)
        ω_n = len(ω)
        assert np.ndim(m_combining_tilde_t_comma_b) == 0
        assert np.ndim(β_b) == 0
        assert np.ndim(σ_P_circumflex_accent_A) == 0
        assert np.ndim(σ_P_circumflex_accent_B) == 0
        assert np.ndim(σ_O_circumflex_accent_A) == 0
        assert np.ndim(σ_O_circumflex_accent_B) == 0
        self.CSF = CSF
        self.ω = ω
        self.m_combining_tilde_t_comma_b = m_combining_tilde_t_comma_b
        self.β_b = β_b
        self.σ_P_circumflex_accent_A = σ_P_circumflex_accent_A
        self.σ_P_circumflex_accent_B = σ_P_circumflex_accent_B
        self.σ_O_circumflex_accent_A = σ_O_circumflex_accent_A
        self.σ_O_circumflex_accent_B = σ_O_circumflex_accent_B
        self.ω_n = ω_n
        # `$∆Q_P$` = `$E_b$`(`$σ_P^A$`) - `$E_b$`(`$σ_P^B$`)
        self.increment_Q_P = self.E_b(σ_P_circumflex_accent_A) - self.E_b(σ_P_circumflex_accent_B)
        # `$∆Q_O$` = `$E_b$`(`$σ_O^A$`) - `$E_b$`(`$σ_O^B$`)
        self.increment_Q_O = self.E_b(σ_O_circumflex_accent_A) - self.E_b(σ_O_circumflex_accent_B)

    def m(self, ω, σ):
        return np.exp(-((2 * (np.pi**2)) * (ω**2)) * (σ**2))

    def m̃(self, ω, σ):
        return self.CSF(ω) * self.m(ω, σ)

    def E_b(self, σ):
        sum_1 = 0
        for i in range(1, self.ω_n+1):
            sum_1 += (self.m̃(self.ω[i-1], σ) / self.m_combining_tilde_t_comma_b)**self.β_b
        return sum_1

