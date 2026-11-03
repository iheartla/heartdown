function output = computation(lambda_s, E_a, D, S_1, S_2, U_1, U_2, R)
% output = computation(lambda_s, E_a, D, S_1, S_2, U_1, U_2, R)
%
%    E = `$λ_s$``$E_s$` + (1 - `$λ_s$`)`$E_f$` 
%    where
%    `$λ_s$` ∈ ℝ 
%    
%    `$E_s$` = `$E_c$` + `$E_a$` 
%    where
%    `$E_a$` ∈ ℝ
%    
%    `$E_c$` = D(`$S_1$`, `$U_1$`) + D(`$S_2$`, `$U_2$`)
%    where
%    D ∈ ℝ, ℝ -> ℝ
%    `$S_1$` ∈ ℝ
%    `$S_2$` ∈ ℝ
%    `$U_1$` ∈ ℝ
%    `$U_2$` ∈ ℝ
%    
%    `$E_f$` = sum_i ||R_i||
%    where
%    
%    R_i ∈ ℝ^3
    R_n = size(R, 1);
    assert( numel(lambda_s) == 1 );
    assert( numel(E_a) == 1 );
    assert( numel(S_1) == 1 );
    assert( numel(S_2) == 1 );
    assert( numel(U_1) == 1 );
    assert( numel(U_2) == 1 );
    
    % `$E_c$` = D(`$S_1$`, `$U_1$`) + D(`$S_2$`, `$U_2$`)
    E_c = D(S_1, U_1) + D(S_2, U_2);
    % `$E_s$` = `$E_c$` + `$E_a$`
    E_s = E_c + E_a;
    % `$E_f$` = sum_i ||R_i||
    sum_1 = 0;
    for i_ = 1:R_n
        sum_1 = sum_1 + norm(R(i_, :).');
    end
    E_f = sum_1;
    % E = `$λ_s$``$E_s$` + (1 - `$λ_s$`)`$E_f$`
    E = lambda_s * E_s + (1 - lambda_s) * E_f;
    output.E_c = E_c;
    output.E_s = E_s;
    output.E_f = E_f;
    output.E = E;
end

