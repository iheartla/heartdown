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
%    `V'` = (L³ + w diag(B))⁻¹ (w diag(B) V)
%    where
%    B ∈ ℤ^n
%    V ∈ ℝ^(n × 3)
%    
    B = reshape(B,[],1);

    assert(size(E,2) == 2)
    assert(numel(n) == 1);
    assert( numel(B) == n );
    assert( isequal(size(V), [n, 3]) );

    % w=10^6

    w = 10.^6;
    % A_ij = { 1 if (i,j) ∈ E
    %          1 if (j,i) ∈ E
    %          0 otherwise

    Aij_0 = zeros(2,0);
    Avals_0 = zeros(1,0);
    for index = 1:size(E, 1)
        i = E(index, 1);
        j = E(index, 2);
        Aij_0(1:2,end+1) = [i;j];
        Avals_0(end+1) = 1;
    end
    for index_1 = 1:size(E, 1)
        j = E(index_1, 1);
        i = E(index_1, 2);
        Aij_0(1:2,end+1) = [i;j];
        Avals_0(end+1) = 1;
    end
    sparse_0 = sparse(Aij_0(1,:),Aij_0(2,:),Avals_0,n,n);
    A = sparse_0;
    Dii_0 = zeros(2,0);
    Dvals_0 = zeros(1,0);
    for i = 1:n
        sum_0 = 0;
        for j = 1:size(A,2)
            sum_0 = sum_0 + A(i, j);
        end
        Dii_0(1:2,end+1) = [i;i];
        Dvals_0(end+1) = sum_0;
    end
    D = sparse(Dii_0(1,:),Dii_0(2,:),Dvals_0,n,n);
    % L = D⁻¹ ( D - A )

    L = ((D)\((D - A)));
    % `V'` = (L³ + w diag(B))⁻¹ (w diag(B) V)

    V_apostrophe = (((L^3 + w * diag(B)))\((w * diag(B) * V)));
    output.w = w;
    output.A = A;
    output.D = D;
    output.L = L;
    output.V_apostrophe = V_apostrophe;
end

