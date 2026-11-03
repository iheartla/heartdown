function output = judder(F, L, S, P, a, b, M, L_a, L_b)
% output = judder(F, L, S, P, a, b, M, L_a, L_b)
%
%    J = P(α(F), β(L), S)
%    F ∈ ℝ
%    L ∈ ℝ
%    S ∈ ℝ
%    P ∈ ℝ, ℝ, ℝ -> ℝ
%    
%    
%    CFF(L) = a ⋅ log(L) + b where L ∈ ℝ
%    
%    a ∈ ℝ
%    b ∈ ℝ
%    
%    
%    `$F_a$` = M⋅ CFF(`$L_a$`)
%    
%    M ∈ ℝ
%    `$L_a$` ∈ ℝ
%    
%    `$F_b$` = M⋅ CFF(`$L_b$`)
%    
%    `$L_b$` ∈ ℝ
%    
%    α(F) = 1/F where F ∈ ℝ
%    
%    β(L) = log_10(L) where L ∈ ℝ
    assert( numel(F) == 1 );
    assert( numel(L) == 1 );
    assert( numel(S) == 1 );
    assert( numel(a) == 1 );
    assert( numel(b) == 1 );
    assert( numel(M) == 1 );
    assert( numel(L_a) == 1 );
    assert( numel(L_b) == 1 );
    
    % `$F_a$` = M⋅ CFF(`$L_a$`)
    F_a = M * CFF(L_a);
    % `$F_b$` = M⋅ CFF(`$L_b$`)
    F_b = M * CFF(L_b);
    % J = P(α(F), β(L), S)
    J = P(alpha(F), beta(L), S);
    function ret = CFF(L)
        ret = a * log(L) + b;
    end
    function ret = alpha(F)
        ret = 1 / F;
    end
    function ret = beta(L)
        ret = log10(L);
    end
    output.F_a = F_a;
    output.F_b = F_b;
    output.J = J;
end

function output = error(O, M)
% output = error(O, M)
%
%    E = sum_i |log(O_i) - log(M_i)|/log(O_i) 
%    
%    O ∈ ℝ^N
%    M ∈ ℝ^N
    O = reshape(O, [], 1);
    M = reshape(M, [], 1);
    N = size(O, 1);
    assert( numel(O) == N );
    assert( numel(M) == N );
    
    % E = sum_i |log(O_i) - log(M_i)|/log(O_i)
    sum_1 = 0;
    for i_ = 1:N
        sum_1 = sum_1 + abs(log(O(i_)) - log(M(i_))) / log(O(i_));
    end
    E = sum_1;
    output.E = E;
end

function output = third(a, b, F_b, F_a, L_a)
% output = third(a, b, F_b, F_a, L_a)
%
%    
%    `$L_b$` = 10^((a `$F_b$`log((`$L_a$`))+b(`$F_b$`-`$F_a$`))/(a`$F_a$`))
%    a ∈ ℝ
%    b ∈ ℝ
%    `$F_b$` ∈ ℝ
%    `$F_a$` ∈ ℝ
%    `$L_a$` ∈ ℝ
    assert( numel(a) == 1 );
    assert( numel(b) == 1 );
    assert( numel(F_b) == 1 );
    assert( numel(F_a) == 1 );
    assert( numel(L_a) == 1 );
    
    % `$L_b$` = 10^((a `$F_b$`log((`$L_a$`))+b(`$F_b$`-`$F_a$`))/(a`$F_a$`))
    L_b = 10.^(((a * F_b) * log(L_a) + b * (F_b - F_a)) / (a * F_a));
    output.L_b = L_b;
end

