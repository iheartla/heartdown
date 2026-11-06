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
%    `V'` = (L + w diag(B))⁻¹ (w diag(B) V)
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
    rows_1 = zeros(0, 1);
    cols_2 = zeros(0, 1);
    vals_3 = zeros(0, 1);
    for i_ = 1:n
        for j_ = 1:n
            ret_4 = 0;
            if ismember([i_, j_], E, 'rows')
                ret_4 = 1;
            elseif ismember([j_, i_], E, 'rows')
                ret_4 = 1;
            else
                ret_4 = 0;
            end
            value_5 = ret_4;
            if value_5 ~= 0
                rows_1(end+1, 1) = i_;
                cols_2(end+1, 1) = j_;
                vals_3(end+1, 1) = value_5;
            end
        end
    end
    A = sparse(rows_1, cols_2, vals_3, n, n);
    % D_ii = ∑_j A_ij
    rows_6 = zeros(0, 1);
    cols_7 = zeros(0, 1);
    vals_8 = zeros(0, 1);
    for i_ = 1:n
        sum_9 = 0;
        for j_ = 1:n
            sum_9 = sum_9 + A(i_, j_);
        end
        value_10 = sum_9;
        if value_10 ~= 0
            rows_6(end+1, 1) = i_;
            cols_7(end+1, 1) = i_;
            vals_8(end+1, 1) = value_10;
        end
    end
    D = sparse(rows_6, cols_7, vals_8, n, n);
    % L = D⁻¹ ( D - A )
    L = D \ (D - A);
    % `V'` = (L + w diag(B))⁻¹ (w diag(B) V)
    V_apostrophe = (L + w * diag(B)) \ ((w * diag(B)) * V);
    output.w = w;
    output.A = A;
    output.D = D;
    output.L = L;
    output.V_apostrophe = V_apostrophe;
end

