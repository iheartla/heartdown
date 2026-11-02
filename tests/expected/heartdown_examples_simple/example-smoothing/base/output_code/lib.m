function output = MeshSmoothing(V, L, lambda_)
% output = MeshSmoothing(V, L, λ)
%
%    `E_\text{data}`(U) = ‖ U - V ‖² where U ∈ ℝ^(n×3)
%    `E_\text{smoothness}`(U) = trace( Uᵀ L U ) where U ∈ ℝ^(n×3)
%    trace from linearalgebra
%    
%    min_( U ∈ ℝ^(n×3) ) `E_\text{data}`(U) + λ `E_\text{smoothness}`(U)
%    where
%    V ∈ ℝ^(n×3): The vertices of the mesh
%    L ∈ ℝ^(n×n) sparse: The Laplacian matrix
%    λ ∈ ℝ
%    
    n = size(V, 1);
    assert( isequal(size(V), [n, 3]) );
    assert( isequal(size(L), [n, n]) );
    assert(numel(lambda_) == 1);

    U = optimvar('U', n, 3);
    target_2 = @(U) E_textdata(reshape(U, [n, 3])) + lambda_ * E_textsmoothness(reshape(U, [n, 3]));
    [~, optimize] = fminunc(target_2,zeros(n*3,1));
    ret = optimize;
    function [ret_1] = E_textdata(U)
        n = size(U, 1);
        assert( isequal(size(U), [n, 3]) );

        ret_1 = norm(U - V, 'fro').^2;
    end

    function [ret_2] = E_textsmoothness(U)
        n = size(U, 1);
        assert( isequal(size(U), [n, 3]) );

        ret_2 = trace(U' * L * U);
    end

    output.ret = ret;
    output.E_textdata = @E_textdata;
    output.E_textsmoothness = @E_textsmoothness;
    output.V = V;    
    output.L = L;    
    output.lambda_ = lambda_;
end

