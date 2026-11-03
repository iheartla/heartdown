function output = ICP(R, a, theta, p, q, n_q, n_p, t, barq, barp, trans, rot)
% output = ICP(R, a, theta, p, q, n_q, n_p, t, barq, barp, trans, rot)
%
%    ã = a tan(θ)
%    n_i = `$n_q$`_i + `$n_p$`_i
%    t̃ = t/cos(θ)
%     p̃_i = p_i - `$\bar{p}$` 
%     q̃_i = q_i - `$\bar{q}$` 
%    `$\varepsilon_{point}$` = ∑_i ||R p_i + t - q_i||
%    R ∈ ℝ^(3 × 3): the rigid-body rotation matrix
%    
%    `$\varepsilon_{plane}$` = ∑_i ((R p_i + t - q_i) ⋅ `$n_q$`_i)^2
%    
%    `$\varepsilon_{symm-RN}$` = ∑_i ((R p_i + R⁻¹ q_i + t) ⋅ (R`$n_p$`_i + R⁻¹`$n_q$`_i))^2
%    
%    tan, cos from trigonometry
%    
%    `$\varepsilon_{symm}$` = ∑_i cos²(θ)((p_i - q_i)⋅n_i +((p_i+q_i)×n_i)⋅ã+n_i⋅t̃)² 
%    
%    where
%    a ∈ ℝ³ : axis of rotation
%    θ ∈ ℝ  : angle of rotation
%    p_i ∈ ℝ³: a sequence of points
%    q_i ∈ ℝ³: a sequence of points
%    `$n_q$`_i ∈ ℝ³: the surface normals
%    `$n_p$`_i ∈ ℝ³: the surface normals
%    t ∈ ℝ³: the translation vector
%    
%    S = trans(`$\bar{q}$`) ⋅ rot(θ, ã/||ã||) ⋅trans(t̃ cos(θ)) ⋅rot(θ, ã/||ã||)⋅ trans(-`$\bar{p}$`)
%    
%    where 
%    `$\bar{q}$` ∈ ℝ³: the averaged coordinate of points
%    `$\bar{p}$` ∈ ℝ³: the averaged coordinate of points
%    trans ∈ ℝ³ -> ℝ^(4 × 4) : the translation function
%    rot ∈ ℝ, ℝ³ -> ℝ^(4 × 4): the rotation function
%    
%    `$\varepsilon_{two-plane}$` = ∑_i(((R p_i + R⁻¹ q_i + t) ⋅ (R `$n_p$`_i))^2 + ((R p_i + R⁻¹ q_i + t) ⋅ (R⁻¹`$n_q$`_i))^2)
    a = reshape(a, [], 1);
    t = reshape(t, [], 1);
    barq = reshape(barq, [], 1);
    barp = reshape(barp, [], 1);
    p_n = size(p, 1);
    q_n = size(q, 1);
    n_q_n = size(n_q, 1);
    n_p_n = size(n_p, 1);
    assert( isequal(size(R), [3, 3]) );
    assert( numel(a) == 3 );
    assert( numel(theta) == 1 );
    assert( numel(t) == 3 );
    assert( numel(barq) == 3 );
    assert( numel(barp) == 3 );
    
    % ã = a tan(θ)
    a_tilde = a * tan(theta);
    % n_i = `$n_q$`_i + `$n_p$`_i
    n = zeros(n_q_n, 3);
    for i_ = 1:n_q_n
        n(i_, :) = (n_q(i_, :).' + n_p(i_, :).').';
    end
    % t̃ = t/cos(θ)
    t_tilde = t / cos(theta);
    % p̃_i = p_i - `$\bar{p}$`
    p_tilde = zeros(p_n, 3);
    for i_ = 1:p_n
        p_tilde(i_, :) = (p(i_, :).' - barp).';
    end
    % q̃_i = q_i - `$\bar{q}$`
    q_tilde = zeros(q_n, 3);
    for i_ = 1:q_n
        q_tilde(i_, :) = (q(i_, :).' - barq).';
    end
    % `$\varepsilon_{point}$` = ∑_i ||R p_i + t - q_i||
    sum_1 = 0;
    for i_ = 1:p_n
        sum_1 = sum_1 + norm(R * p(i_, :).' + t - q(i_, :).');
    end
    varepsilon_point = sum_1;
    % `$\varepsilon_{plane}$` = ∑_i ((R p_i + t - q_i) ⋅ `$n_q$`_i)^2
    sum_2 = 0;
    for i_ = 1:p_n
        sum_2 = sum_2 + dot(R * p(i_, :).' + t - q(i_, :).', n_q(i_, :).').^2;
    end
    varepsilon_plane = sum_2;
    % `$\varepsilon_{symm-RN}$` = ∑_i ((R p_i + R⁻¹ q_i + t) ⋅ (R`$n_p$`_i + R⁻¹`$n_q$`_i))^2
    sum_3 = 0;
    for i_ = 1:p_n
        sum_3 = sum_3 + dot(R * p(i_, :).' + R \ q(i_, :).' + t, R * n_p(i_, :).' + R \ n_q(i_, :).').^2;
    end
    varepsilon_symmRN = sum_3;
    % `$\varepsilon_{symm}$` = ∑_i cos²(θ)((p_i - q_i)⋅n_i +((p_i+q_i)×n_i)⋅ã+n_i⋅t̃)²
    sum_4 = 0;
    for i_ = 1:p_n
        sum_4 = sum_4 + (cos(theta).^2) * ((dot(p(i_, :).' - q(i_, :).', n(i_, :).') + dot(cross(p(i_, :).' + q(i_, :).', n(i_, :).'), a_tilde) + dot(n(i_, :).', t_tilde)).^2);
    end
    varepsilon_symm = sum_4;
    % S = trans(`$\bar{q}$`) ⋅ rot(θ, ã/||ã||) ⋅trans(t̃ cos(θ)) ⋅rot(θ, ã/||ã||)⋅ trans(-`$\bar{p}$`)
    S = (((trans(barq) * rot(theta, a_tilde / norm(a_tilde))) * trans(t_tilde * cos(theta))) * rot(theta, a_tilde / norm(a_tilde))) * trans(-barp);
    % `$\varepsilon_{two-plane}$` = ∑_i(((R p_i + R⁻¹ q_i + t) ⋅ (R `$n_p$`_i))^2 + ((R p_i + R⁻¹ q_i + t) ⋅ (R⁻¹`$n_q$`_i))^2)
    sum_5 = 0;
    for i_ = 1:p_n
        sum_5 = sum_5 + dot(R * p(i_, :).' + R \ q(i_, :).' + t, R * n_p(i_, :).').^2 + dot(R * p(i_, :).' + R \ q(i_, :).' + t, R \ n_q(i_, :).').^2;
    end
    varepsilon_twoplane = sum_5;
    output.a_tilde = a_tilde;
    output.n = n;
    output.t_tilde = t_tilde;
    output.p_tilde = p_tilde;
    output.q_tilde = q_tilde;
    output.varepsilon_point = varepsilon_point;
    output.varepsilon_plane = varepsilon_plane;
    output.varepsilon_symmRN = varepsilon_symmRN;
    output.varepsilon_symm = varepsilon_symm;
    output.S = S;
    output.varepsilon_twoplane = varepsilon_twoplane;
end

