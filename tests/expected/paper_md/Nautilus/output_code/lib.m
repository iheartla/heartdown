function output = nautilus(x, lambda_, v)
% output = nautilus(x, lambda_, v)
%
%    H = PSP⁻¹
%    
%    P = [1 x₁ 0
%         0 x₂ 0
%         x₃ x₄ 1]
%    S = [x₅ x₆ x₇
%         x₈ x₉ x₁₀
%         0 0 1]
%    where
%    x ∈ ℝ^n
%    
%    E(x) = λ||(x₅, x₈) - (x₉, x₆)||^2 + sum_i ||H v_i - PSP⁻¹ v_i||^2 where x ∈ ℝ^n
%    
%    where
%    λ ∈ ℝ
%    v_i ∈ ℝ^(3 × 3)
    x = reshape(x, [], 1);
    n = size(x, 1);
    v_n = size(v, 1);
    assert( numel(x) == n );
    assert( numel(lambda_) == 1 );
    
    % P = [1 x₁ 0
    %      0 x₂ 0
    %      x₃ x₄ 1]
    P = [[1, x(1), 0]; [0, x(2), 0]; [x(3), x(4), 1]];
    % S = [x₅ x₆ x₇
    %      x₈ x₉ x₁₀
    %      0 0 1]
    S = [[x(5), x(6), x(7)]; [x(8), x(9), x(10)]; [0, 0, 1]];
    % H = PSP⁻¹
    H = (P * S) * inv(P);
    function ret = E(x)
        x = reshape(x, [], 1);
        sum_1 = 0;
        for i_ = 1:v_n
            sum_1 = sum_1 + norm(full(H * reshape(v(i_, :, :), 3, 3) - ((P * S) * P \ reshape(v(i_, :, :), 3, 3))), 'fro').^2;
        end
        ret = lambda_ * (norm([x(5); x(8)] - [x(9); x(6)]).^2) + sum_1;
    end
    output.P = P;
    output.S = S;
    output.H = H;
end

