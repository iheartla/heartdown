function output = siere(U_s, M, v, f, K, boldsymbolI, h, phi_1, u)
% output = siere(U_s, M, v, f, K, boldsymbolI, h, phi_1, u)
%
%    `$G(u)$` = [`$v_G$`
%              M⁻¹`$f_G$`]
%    `$H(u)$` = [`$v_H$`
%              M⁻¹`$f_H$`]
%    `$v_G$` = `$U_s$``$U_s$`^T Mv
%    `$v_H$` =  v - `$v_G$`
%    `$f_G$` =  M`$U_s$``$U_s$`^T f
%    `$f_H$` =  f - `$f_G$`
%    
%    where 
%    `$U_s$` : ℝ^(n × s)
%    M : ℝ^(n × n)
%    v : ℝ^n
%    f : ℝ^n
%    K : ℝ^(n × n)
%    
%    `$J_G$` = [0    `$U_s$``$U_s$`^TM
%          -`$U_s$``$U_s$`^TK`$U_s$``$U_s$`^TM 0 ]
%    `$J_H$` =  [0     I_n
%                -M⁻¹K 0] - `$J_G$` 
%    
%    
%    `$u_+$` =  u + (`$\boldsymbol{I}$` -h`$J_H$`)⁻¹(h `$H(u)$` + h[`$U_s$` 0
%                                                   0   `$U_s$`] `$φ_1$`(h`$J_G^r$`) `$G^r(u)$`)
%    where 
%    `$\boldsymbol{I}$` : ℝ^(2n × 2n)
%    h : ℝ
%    `$φ_1$` : ℝ^(k×k) -> ℝ^(k×k)
%    u : ℝ^(2n × 1)
%    
%    `$J_G^r$` = [0     I_s
%                -`$U_s$`^TK`$U_s$` 0]
%    `$G^r(u)$` = [`$U_s$`^TMv
%                `$U_s$`^Tf]
    v = reshape(v, [], 1);
    f = reshape(f, [], 1);
    n = size(U_s, 1);
    s = size(U_s, 2);
    assert( isequal(size(U_s), [n, s]) );
    assert( isequal(size(M), [n, n]) );
    assert( numel(v) == n );
    assert( numel(f) == n );
    assert( isequal(size(K), [n, n]) );
    assert( isequal(size(boldsymbolI), [2*n, 2*n]) );
    assert( numel(h) == 1 );
    assert( isequal(size(u), [2*n, 1]) );
    
    % `$v_G$` = `$U_s$``$U_s$`^T Mv
    v_G = ((U_s * U_s.') * M) * v;
    % `$v_H$` =  v - `$v_G$`
    v_H = v - v_G;
    % `$f_G$` =  M`$U_s$``$U_s$`^T f
    f_G = ((M * U_s) * U_s.') * f;
    % `$G(u)$` = [`$v_G$`
    %           M⁻¹`$f_G$`]
    Gu = [[v_G]; [M \ f_G]];
    % `$f_H$` =  f - `$f_G$`
    f_H = f - f_G;
    % `$H(u)$` = [`$v_H$`
    %           M⁻¹`$f_H$`]
    Hu = [[v_H]; [M \ f_H]];
    % `$J_G$` = [0    `$U_s$``$U_s$`^TM
    %       -`$U_s$``$U_s$`^TK`$U_s$``$U_s$`^TM 0 ]
    J_G = [[zeros(n, n), (U_s * U_s.') * M]; [-(((((U_s * U_s.') * K) * U_s) * U_s.') * M), zeros(n, n)]];
    % `$J_H$` =  [0     I_n
    %             -M⁻¹K 0] - `$J_G$`
    J_H = [[zeros(n, n), eye(n)]; [-M \ K, zeros(n, n)]] - J_G;
    % `$J_G^r$` = [0     I_s
    %             -`$U_s$`^TK`$U_s$` 0]
    J_G_r = [[zeros(s, s), eye(s)]; [-((U_s.' * K) * U_s), zeros(s, s)]];
    % `$G^r(u)$` = [`$U_s$`^TMv
    %             `$U_s$`^Tf]
    G_ru = [[(U_s.' * M) * v]; [U_s.' * f]];
    % `$u_+$` =  u + (`$\boldsymbol{I}$` -h`$J_H$`)⁻¹(h `$H(u)$` + h[`$U_s$` 0
    %                                                0   `$U_s$`] `$φ_1$`(h`$J_G^r$`) `$G^r(u)$`)
    u_plus_sign = u + (boldsymbolI - (h * J_H)) \ (h * Hu + ((h * [[U_s, zeros(n, s)]; [zeros(n, s), U_s]]) * phi_1(h * J_G_r)) * G_ru);
    output.v_G = v_G;
    output.v_H = v_H;
    output.f_G = f_G;
    output.Gu = Gu;
    output.f_H = f_H;
    output.Hu = Hu;
    output.J_G = J_G;
    output.J_H = J_H;
    output.J_G_r = J_G_r;
    output.G_ru = G_ru;
    output.u_plus_sign = u_plus_sign;
end

function output = second(U_s, M, K)
% output = second(U_s, M, K)
%
%    `$J_G$` = `$Y_1$``$Z_1$`^T + `$Y_2$``$Z_2$`^T 
%    
%    `$Y_1$` =  [`$U_s$`
%                 0]
%    `$Z_1$` =  [ 0
%                 M`$U_s$`] 
%    `$Y_2$` =  [0
%                -`$U_s$``$U_s$`^TK`$U_s$`]
%    `$Z_2$` =  [ M`$U_s$`
%                 0] 
%    
%    where 
%    `$U_s$` : ℝ^(n × s)
%    M : ℝ^(n × n) 
%    K : ℝ^(n × n)
    n = size(U_s, 1);
    s = size(U_s, 2);
    assert( isequal(size(U_s), [n, s]) );
    assert( isequal(size(M), [n, n]) );
    assert( isequal(size(K), [n, n]) );
    
    % `$Y_1$` =  [`$U_s$`
    %              0]
    Y_1 = [[U_s]; [zeros(1, s)]];
    % `$Z_1$` =  [ 0
    %              M`$U_s$`]
    Z_1 = [[zeros(1, s)]; [M * U_s]];
    % `$Y_2$` =  [0
    %             -`$U_s$``$U_s$`^TK`$U_s$`]
    Y_2 = [[zeros(1, s)]; [-(((U_s * U_s.') * K) * U_s)]];
    % `$Z_2$` =  [ M`$U_s$`
    %              0]
    Z_2 = [[M * U_s]; [zeros(1, s)]];
    % `$J_G$` = `$Y_1$``$Z_1$`^T + `$Y_2$``$Z_2$`^T
    J_G = Y_1 * Z_1.' + Y_2 * Z_2.';
    output.Y_1 = Y_1;
    output.Z_1 = Z_1;
    output.Y_2 = Y_2;
    output.Z_2 = Z_2;
    output.J_G = J_G;
end

