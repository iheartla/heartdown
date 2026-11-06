import numpy as np
import scipy
import scipy.linalg
from scipy import sparse
from scipy.integrate import quad, solve_ivp
from scipy.optimize import minimize


class Anisotropic:
    def __init__(self, a, C, F):
        a = np.asarray(a, dtype=np.float64)
        C = np.asarray(C, dtype=np.float64)
        F = np.asarray(F, dtype=np.float64)
        assert a.shape == (3,)
        assert C.shape == (3, 3)
        assert F.shape == (3, 3)
        self.a = a
        self.C = C
        self.F = F
        # A = a a^T
        self.A = (a).reshape(-1, 1) @ (a).reshape(1, -1)
        # `$\lambda_{0,1,2}$`=2||a||_2^2
        self.lambda_0_comma_1_comma_2 = 2 * (np.power(np.linalg.norm(a, 2), 2))
        # `$I_5$` = tr(CA)
        self.I_5 = np.trace(C @ self.A)
        # `$\frac{∂I₅}{∂F}$` = 2FA
        self.fracdeeI5deeF = (2 * F) @ self.A
        # `$\frac{∂²I₅}{∂f²}$` = 2[A₁,₁I₃  A₁,₂I₃  A₁,₃I₃
        #                A₂,₁I₃  A₂,₂I₃  A₂,₃I₃
        #                A₃,₁I₃  A₃,₂I₃  A₃,₃I₃]
        self.fracdee2I5deef2 = 2 * np.block([[self.A[0, 0] * np.identity(3), self.A[0, 1] * np.identity(3), self.A[0, 2] * np.identity(3)], [self.A[1, 0] * np.identity(3), self.A[1, 1] * np.identity(3), self.A[1, 2] * np.identity(3)], [self.A[2, 0] * np.identity(3), self.A[2, 1] * np.identity(3), self.A[2, 2] * np.identity(3)]]).astype(np.float64)
        # `$\mathbf{Q}_{0}$` = [a^T
        #                0
        #                0]
        self.mathbfQ_0 = np.block([[(a).reshape(1, -1)], [np.zeros((1, 3))], [np.zeros((1, 3))]]).astype(np.float64)
        # `$\mathbf{Q}_{1}$` = [0
        #                     a^T
        #                0]
        self.mathbfQ_1 = np.block([[np.zeros((1, 3))], [(a).reshape(1, -1)], [np.zeros((1, 3))]]).astype(np.float64)
        # `$\mathbf{Q}_{2}$` = [ 0
        #                0
        #                a^T]
        self.mathbfQ_2 = np.block([[np.zeros((1, 3))], [np.zeros((1, 3))], [(a).reshape(1, -1)]]).astype(np.float64)

class Anisotropic2D:
    def __init__(self, a, A):
        a = np.asarray(a, dtype=np.float64)
        A = np.asarray(A, dtype=np.float64)
        assert a.shape == (2,)
        assert A.shape == (2, 2)
        self.a = a
        self.A = A
        # `$\lambda_{0,1}$`=2||a||_2^2
        self.lambda_0_comma_1 = 2 * (np.power(np.linalg.norm(a, 2), 2))
        # `$\frac{∂²I₅}{∂f²}$` = 2[A₁,₁I_2  A₁,₂I_2
        #                A₂,₁I_2  A₂,₂I_2]
        self.fracdee2I5deef2 = 2 * np.block([[A[0, 0] * np.identity(2), A[0, 1] * np.identity(2)], [A[1, 0] * np.identity(2), A[1, 1] * np.identity(2)]]).astype(np.float64)
        # `$\mathbf{Q}_{0}$` = [a^T
        #                0]
        self.mathbfQ_0 = np.block([[(a).reshape(1, -1)], [np.zeros((1, 2))]]).astype(np.float64)
        # `$\mathbf{Q}_{1}$` = [0
        #                     a^T]
        self.mathbfQ_1 = np.block([[np.zeros((1, 2))], [(a).reshape(1, -1)]]).astype(np.float64)

