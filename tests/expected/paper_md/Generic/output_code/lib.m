function output = Generic(J, x, y, u, v)
% output = Generic(J, x, y, u, v)
%
%     `$x_p$` = (-y, x)
%     `$v_p$` = (-v, u)
%    M=[-J`$x_p$`+`$v_p$`  J  `$x_p$`  I_2] 
%    
%    where
%    J: ℝ^(2×2): The Jacobian matrix
%    x: ℝ : point coordinate
%    y: ℝ : point coordinate
%    u: ℝ : vector field coordinate
%    v: ℝ : vector field coordinate
    assert( isequal(size(J), [2, 2]) );
    assert( numel(x) == 1 );
    assert( numel(y) == 1 );
    assert( numel(u) == 1 );
    assert( numel(v) == 1 );
    
    % `$x_p$` = (-y, x)
    x_p = [-y; x];
    % `$v_p$` = (-v, u)
    v_p = [-v; u];
    % M=[-J`$x_p$`+`$v_p$`  J  `$x_p$`  I_2]
    M = [[-(J * x_p) + v_p, J, x_p, eye(2)]];
    output.x_p = x_p;
    output.v_p = v_p;
    output.M = M;
end

