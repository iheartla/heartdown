function output = perceptual(CSF, omega, m_tilde_t_b, beta_b, sigma_P_A, sigma_P_B, sigma_O_A, sigma_O_B)
% output = perceptual(CSF, omega, m_tilde_t_b, beta_b, sigma_P_A, sigma_P_B, sigma_O_A, sigma_O_B)
%
%    m(ω;σ) = exp(-2π^2 ω^2 σ^2) where ω ∈ ℝ, σ∈ ℝ
%    
%    m̃(ω;σ) = CSF(ω) m(ω;σ) where ω ∈ ℝ, σ∈ ℝ
%    where
%    CSF ∈ ℝ->ℝ
%    
%    
%    `$E_b$`(σ) = sum_i ( (m̃(ω_i ; σ))/`$m̃_{t,b}$`)^`$β_b$`  where σ ∈ ℝ
%    where
%    ω_i ∈ ℝ
%    `$m̃_{t,b}$` ∈ ℝ
%    `$β_b$` ∈ ℝ
%    
%    `$∆Q_P$` = `$E_b$`(`$σ_P^A$`) - `$E_b$`(`$σ_P^B$`)
%    `$∆Q_O$` = `$E_b$`(`$σ_O^A$`) - `$E_b$`(`$σ_O^B$`)
%    where
%    `$σ_P^A$` ∈ ℝ
%    `$σ_P^B$` ∈ ℝ
%    `$σ_O^A$` ∈ ℝ
%    `$σ_O^B$` ∈ ℝ
    omega = reshape(omega, [], 1);
    omega_n = size(omega, 1);
    assert( numel(m_tilde_t_b) == 1 );
    assert( numel(beta_b) == 1 );
    assert( numel(sigma_P_A) == 1 );
    assert( numel(sigma_P_B) == 1 );
    assert( numel(sigma_O_A) == 1 );
    assert( numel(sigma_O_B) == 1 );
    
    % `$∆Q_P$` = `$E_b$`(`$σ_P^A$`) - `$E_b$`(`$σ_P^B$`)
    increment_Q_P = E_b(sigma_P_A) - E_b(sigma_P_B);
    % `$∆Q_O$` = `$E_b$`(`$σ_O^A$`) - `$E_b$`(`$σ_O^B$`)
    increment_Q_O = E_b(sigma_O_A) - E_b(sigma_O_B);
    function ret = m(omega, sigma)
        ret = exp(-(((2 * (pi.^2)) * (omega.^2)) * (sigma.^2)));
    end
    function ret = m_tilde(omega, sigma)
        ret = CSF(omega) * m(omega, sigma);
    end
    function ret = E_b(sigma)
        sum_1 = 0;
        for i_ = 1:omega_n
            sum_1 = sum_1 + (m_tilde(omega(i_), sigma) / m_tilde_t_b).^beta_b;
        end
        ret = sum_1;
    end
    output.increment_Q_P = increment_Q_P;
    output.increment_Q_O = increment_Q_O;
end

