function output = clustering(y, w)
% output = clustering(y, w)
%
%    min_( c ∈ ℝ^m ) ∑_i w_i ‖ y_i - c ‖^2
%    where
%    y_i ∈ ℝ^m
%    w_i ∈ ℝ
%    
    w = reshape(w,[],1);

    dim_0 = size(w, 1);
    m = size(y, 2);
    assert( isequal(size(y), [dim_0, m]) );
    assert( size(w,1) == dim_0 );

    c = optimvar('c', m);
    function ret = target_2(c)
        sum_0 = 0;
        for i = 1:size(w, 1)
            sum_0 = sum_0 + w(i) * norm(y(i,:)' - c, 2).^2;
        end
        ret = sum_0;
    end
    [~, optimize] = fminunc(@target_2,zeros(m,1));
    ret = optimize;
    output.ret = ret;
    output.w = w;    
    output.y = y;
end

