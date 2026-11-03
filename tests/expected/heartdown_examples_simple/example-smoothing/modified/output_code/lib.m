function output = MeshSmoothing(V, L, lambda_)
% output = MeshSmoothing(V, L, lambda_)
%
%    `E_\text{data}`(U) = ‖ U - V ‖² where U ∈ ℝ^(n×3)
%    `E_\text{smoothness}`(U) = trace( Uᵀ L U ) where U ∈ ℝ^(n×3)
%    trace from linearalgebra
%    
%    min_( U ∈ ℝ^(n×3) ) (1-λ) `E_\text{data}`(U) + λ `E_\text{smoothness}`(U)
%    where
%    V ∈ ℝ^(n×3): The vertices of the mesh
%    L ∈ ℝ^(n×n) sparse: The Laplacian matrix
%    λ ∈ ℝ
    n = size(V, 1);
    assert( isequal(size(V), [n, 3]) );
    assert( numel(lambda_) == 1 );
    
    % min_( U ∈ ℝ^(n×3) ) (1-λ) `E_\text{data}`(U) + λ `E_\text{smoothness}`(U)
    n_2 = 0 + (n)*(3);
    lb_3 = -Inf(n_2, 1); ub_4 = Inf(n_2, 1);
    function x_7 = tobox_5(z_6)
        x_7 = z_6;
        m_8 = isfinite(lb_3) & isfinite(ub_4); x_7(m_8) = lb_3(m_8) + (ub_4(m_8) - lb_3(m_8)) .* (sin(z_6(m_8)) + 1) / 2;
        m_8 = isfinite(lb_3) & ~isfinite(ub_4); x_7(m_8) = lb_3(m_8) + z_6(m_8).^2;
        m_8 = ~isfinite(lb_3) & isfinite(ub_4); x_7(m_8) = ub_4(m_8) - z_6(m_8).^2;
    end
    function value_11 = objective_9(z_12)
        X_10 = tobox_5(z_12);
        U = reshape(X_10(0 + 1:0 + (n)*(3)), n, 3);
        value_11 = ((1 - lambda_) * E_textdata(U) + lambda_ * E_textsmoothness(U));
    end
    z_13 = fminsearch(@objective_9, zeros(n_2, 1), optimset('TolX', 1e-12, 'TolFun', 1e-12, 'MaxFunEvals', 1e5, 'MaxIter', 1e5));
    opt_1 = objective_9(z_13);
    ret = opt_1;
    function ret = E_textdata(U)
        ret = norm(full(U - V), 'fro').^2;
    end
    function ret = E_textsmoothness(U)
        ret = trace((U.' * L) * U);
    end
    output.ret = ret;
end

