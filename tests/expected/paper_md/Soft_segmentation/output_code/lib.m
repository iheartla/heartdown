function output = soft(D, boldsymbolu, sigma, alpha)
% output = soft(D, boldsymbolu, sigma, alpha)
%
%    `$F_S$` = sum_i α_i D_i(`$\boldsymbol{u}$`_i) + σ((sum_i α_i)/(sum_i α_i^2) - 1)
%    
%    where
%    
%    D_i: ℝ^3 -> ℝ
%    `$\boldsymbol{u}$`_i: ℝ^3
%    σ: ℝ
%    α_i: ℝ
    alpha = reshape(alpha, [], 1);
    boldsymbolu_n = size(boldsymbolu, 1);
    alpha_n = size(alpha, 1);
    assert( numel(sigma) == 1 );
    
    % `$F_S$` = sum_i α_i D_i(`$\boldsymbol{u}$`_i) + σ((sum_i α_i)/(sum_i α_i^2) - 1)
    sum_1 = 0;
    for i_ = 1:alpha_n
        sum_1 = sum_1 + alpha(i_) * D(i_, boldsymbolu(i_, :).');
    end
    sum_2 = 0;
    for i_ = 1:alpha_n
        sum_2 = sum_2 + alpha(i_);
    end
    sum_3 = 0;
    for i_ = 1:alpha_n
        sum_3 = sum_3 + alpha(i_).^2;
    end
    F_S = sum_1 + sigma * (sum_2 / sum_3 - 1);
    output.F_S = F_S;
end

