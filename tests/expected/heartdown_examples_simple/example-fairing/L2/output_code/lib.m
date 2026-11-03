function output = fairing(E, n, B, V)
% output = fairing(E, n, B, V)
%
%    w=10^6
%    A_ij = { 1 if (i,j) ∈ E
%             1 if (j,i) ∈ E
%             0 otherwise
%    D_ii = ∑_j A_ij
%    L = D⁻¹ ( D - A )
%    where
%    E ∈ { ℤ×ℤ } index
%    A ∈ ℝ^(n×n): The adjacency matrix
%    n ∈ ℤ: The number of mesh vertices
%    
%    diag from linearalgebra
%    
%    `V'` = (L² + w diag(B))⁻¹ (w diag(B) V)
%    where
%    B ∈ ℤ^n
%    V ∈ ℝ^(n × 3)
    B = reshape(B, [], 1);
    assert( numel(n) == 1 );
    assert( numel(B) == n );
    assert( isequal(size(V), [n, 3]) );
    
    % w=10^6
    w = 10.^6;
    % A_ij = { 1 if (i,j) ∈ E
    %          1 if (j,i) ∈ E
    %          0 otherwise
    dense_1 = zeros(n, n);
    for i_ = 1:n
        for j_ = 1:n
            ret_2 = 0;
            if ismember([i_, j_], E, 'rows')
                ret_2 = 1;
            elseif ismember([j_, i_], E, 'rows')
                ret_2 = 1;
            else
                ret_2 = 0;
            end
            dense_1(i_, j_) = ret_2;
        end
    end
    A = sparse(dense_1);
    % D_ii = ∑_j A_ij
    dense_3 = zeros(n, n);
    for i_ = 1:n
        sum_4 = 0;
        for j_ = 1:n
            sum_4 = sum_4 + A(i_, j_);
        end
        dense_3(i_, i_) = sum_4;
    end
    D = sparse(dense_3);
    % L = D⁻¹ ( D - A )
    L = D \ (D - A);
    % `V'` = (L² + w diag(B))⁻¹ (w diag(B) V)
    V_apostrophe = (L^2 + w * diag(B)) \ ((w * diag(B)) * V);
    output.w = w;
    output.A = A;
    output.D = D;
    output.L = L;
    output.V_apostrophe = V_apostrophe;
end

