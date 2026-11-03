function output = clustering(y, w)
% output = clustering(y, w)
%
%    min_( c ∈ ℝ^m ) ∑_i w_i ‖ y_i - c ‖^2
%    where
%    y_i ∈ ℝ^m
%    w_i ∈ ℝ
    w = reshape(w, [], 1);
    y_n = size(y, 1);
    m = size(y, 2);
    w_n = size(w, 1);
    
    % min_( c ∈ ℝ^m ) ∑_i w_i ‖ y_i - c ‖^2
    n_2 = 0 + (m)*(1);
    lb_3 = -Inf(n_2, 1); ub_4 = Inf(n_2, 1);
    function x_7 = tobox_5(z_6)
        x_7 = z_6;
        m_8 = isfinite(lb_3) & isfinite(ub_4); x_7(m_8) = lb_3(m_8) + (ub_4(m_8) - lb_3(m_8)) .* (sin(z_6(m_8)) + 1) / 2;
        m_8 = isfinite(lb_3) & ~isfinite(ub_4); x_7(m_8) = lb_3(m_8) + z_6(m_8).^2;
        m_8 = ~isfinite(lb_3) & isfinite(ub_4); x_7(m_8) = ub_4(m_8) - z_6(m_8).^2;
    end
    function value_11 = objective_9(z_13)
        X_10 = tobox_5(z_13);
        c = reshape(X_10(0 + 1:0 + (m)*(1)), m, 1);
        sum_12 = 0;
        for i_ = 1:w_n
            sum_12 = sum_12 + w(i_) * (norm(y(i_, :).' - c).^2);
        end
        value_11 = (sum_12);
    end
    z_14 = fminsearch(@objective_9, zeros(n_2, 1), optimset('TolX', 1e-12, 'TolFun', 1e-12, 'MaxFunEvals', 1e5, 'MaxIter', 1e5));
    opt_1 = objective_9(z_14);
    ret = opt_1;
    output.ret = ret;
end

