function output = eccentricity(m, italic_p, italic_f_italic_s_0, a)
% output = eccentricity(m, 𝑝, `$𝑓_{𝑠₀}$`, a)
%
%     𝑙(𝐿) = π𝑑(𝐿)^2/4 ⋅ 𝐿 where 𝐿 : ℝ
%    `$𝑙_0$` = 1488
%    q = (5.71 ⋅ 10^(-6), -1.78 ⋅ 10^(-4), 0.204)
%    cos, sin from trigonometry
%    
%    𝑔(x,`$x_0$`, 𝜃,𝜎,`$𝑓_𝑠$`) = exp(-||x-`$x_0$`||^2/(2𝜎^2)) cos(2π`$𝑓_𝑠$`x ⋅(cos(𝜃),sin(𝜃))) where x: ℝ^2,`$x_0$`: ℝ^2,`$𝑓_𝑠$`: ℝ, 𝜎 : ℝ, 𝜃 : ℝ
%    
%    Ψ(𝑒, `$𝑓_𝑠$`)= m(0, 𝑝₁ 𝜏(`$𝑓_𝑠$`)^2 +𝑝₂ 𝜏(`$𝑓_𝑠$`)+𝑝₃ + (𝑝₄ 𝜏(`$𝑓_𝑠$`)^2 + 𝑝₅ 𝜏(`$𝑓_𝑠$`) +𝑝₆)⋅ 𝜁(`$𝑓_𝑠$`)𝑒 + (𝑝₇ 𝜏(`$𝑓_𝑠$`)^2 +𝑝₈ 𝜏(`$𝑓_𝑠$`) + 𝑝₉)⋅𝜁(`$𝑓_𝑠$`)𝑒^2) where `$𝑓_𝑠$` : ℝ, 𝑒: ℝ
%    
%    𝜁(`$𝑓_𝑠$`) = exp(𝑝₁₀ 𝜏(`$𝑓_𝑠$`)) - 1 where `$𝑓_𝑠$` : ℝ
%    𝜏(`$𝑓_𝑠$`) = m(log_10(`$𝑓_𝑠$`)-log_10(`$𝑓_{𝑠₀}$`), 0) where `$𝑓_𝑠$` : ℝ
%    where
%    m: ℝ, ℝ -> ℝ
%    𝑝: ℝ^10: the model parameters
%    `$𝑓_{𝑠₀}$`: ℝ
%    
%    𝐴(𝑒)= ln(64) 2.3/(0.106⋅(𝑒+2.3) )  where 𝑒 : ℝ
%    
%    
%    𝑑(𝐿)= 7.75-5.75((𝐿a/846)^0.41/((𝐿a/846)^0.41 + 2))  where 𝐿 : ℝ
%    where 
%    a : ℝ
%    
%    
%     `$\hat{Ψ}$`(𝑒, `$𝑓_𝑠$`, 𝐿) = (𝑠(𝑒, `$𝑓_𝑠$`) ⋅ (log_10(𝑙(𝐿)/`$𝑙_0$`)) + 1) Ψ(𝑒, `$𝑓_𝑠$`) where `$𝑓_𝑠$` : ℝ, 𝑒: ℝ, 𝐿 : ℝ
%    
%    
%    𝑠(𝑒,`$𝑓_𝑠$`) = 𝜁(`$𝑓_𝑠$`)(q_1 𝑒^2 + q_2 𝑒) + q_3  where `$𝑓_𝑠$` : ℝ, 𝑒: ℝ
%    
    italic_p = reshape(italic_p,[],1);

    assert( numel(italic_p) == 10 );
    assert(numel(italic_f_italic_s_0) == 1);
    assert(numel(a) == 1);

    % `$𝑙_0$` = 1488

    italic_l_0 = 1488;
    % q = (5.71 ⋅ 10^(-6), -1.78 ⋅ 10^(-4), 0.204)

    q = [5.71 * 10.^(-6); -1.78 * 10.^(-4); 0.204];
    function [ret_1] = italic_g(x, x_0, italic_theta, italic_sigma, italic_f_italic_s)
        x = reshape(x,[],1);
        x_0 = reshape(x_0,[],1);
        assert( numel(x) == 2 );
        assert( numel(x_0) == 2 );
        assert(numel(italic_theta) == 1);
        assert(numel(italic_sigma) == 1);
        assert(numel(italic_f_italic_s) == 1);

        ret_1 = exp(-norm(x - x_0, 2).^2 / (2 * italic_sigma.^2)) * cos(dot(2 * pi * italic_f_italic_s * x,[cos(italic_theta); sin(italic_theta)]));
    end

    function [ret_2] = italic_tau(italic_f_italic_s)
        assert(numel(italic_f_italic_s) == 1);

        ret_2 = m(log10(italic_f_italic_s) - log10(italic_f_italic_s_0), 0);
    end

    function [ret_3] = italic_zeta(italic_f_italic_s)
        assert(numel(italic_f_italic_s) == 1);

        ret_3 = exp(italic_p(10) * italic_tau(italic_f_italic_s)) - 1;
    end

    function [ret_4] = Psi(italic_e, italic_f_italic_s)
        assert(numel(italic_e) == 1);
        assert(numel(italic_f_italic_s) == 1);

        ret_4 = m(0, italic_p(1) * italic_tau(italic_f_italic_s).^2 + italic_p(2) * italic_tau(italic_f_italic_s) + italic_p(3) + (italic_p(4) * italic_tau(italic_f_italic_s).^2 + italic_p(5) * italic_tau(italic_f_italic_s) + italic_p(6)) * italic_zeta(italic_f_italic_s) * italic_e + (italic_p(7) * italic_tau(italic_f_italic_s).^2 + italic_p(8) * italic_tau(italic_f_italic_s) + italic_p(9)) * italic_zeta(italic_f_italic_s) * italic_e.^2);
    end

    function [ret_5] = italic_A(italic_e)
        assert(numel(italic_e) == 1);

        ret_5 = log(64) * 2.3 / (0.106 * (italic_e + 2.3));
    end

    function [ret_6] = italic_d(italic_L)
        assert(numel(italic_L) == 1);

        ret_6 = 7.75 - 5.75 * ((italic_L * a / 846).^0.41 / ((italic_L * a / 846).^0.41 + 2));
    end

    function [ret_7] = italic_l(italic_L)
        assert(numel(italic_L) == 1);

        ret_7 = pi * italic_d(italic_L).^2 / 4 * italic_L;
    end

    function [ret_8] = italic_s(italic_e, italic_f_italic_s)
        assert(numel(italic_e) == 1);
        assert(numel(italic_f_italic_s) == 1);

        ret_8 = italic_zeta(italic_f_italic_s) * (q(1) * italic_e.^2 + q(2) * italic_e) + q(3);
    end

    function [ret_9] = hatPsi(italic_e, italic_f_italic_s, italic_L)
        assert(numel(italic_e) == 1);
        assert(numel(italic_f_italic_s) == 1);
        assert(numel(italic_L) == 1);

        ret_9 = (italic_s(italic_e, italic_f_italic_s) * (log10(italic_l(italic_L) / italic_l_0)) + 1) * Psi(italic_e, italic_f_italic_s);
    end

    output.italic_l_0 = italic_l_0;
    output.q = q;
    output.italic_l = @italic_l;
    output.italic_g = @italic_g;
    output.Psi = @Psi;
    output.italic_zeta = @italic_zeta;
    output.italic_tau = @italic_tau;
    output.italic_A = @italic_A;
    output.italic_d = @italic_d;
    output.hatPsi = @hatPsi;
    output.italic_s = @italic_s;
    output.m = m;    
    output.italic_f_italic_s_0 = italic_f_italic_s_0;    
    output.italic_p = italic_p;    
    output.a = a;
end

