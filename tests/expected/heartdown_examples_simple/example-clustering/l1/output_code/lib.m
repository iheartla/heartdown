function output = clustering(y)
% output = clustering(y)
%
%    min_( c ∈ ℝ^m ) ∑_i ‖ y_i - c ‖^1
%    where
%    y_i ∈ ℝ^m
%    
    dim_0 = size(y, 1);
    m = size(y, 2);
    assert( isequal(size(y), [dim_0, m]) );

    c = optimvar('c', m);
    function ret = target_2(c)
        sum_0 = 0;
        for i = 1:size(y, 1)
            sum_0 = sum_0 + norm(y(i,:)' - c, 2).^1;
        end
        ret = sum_0;
    end
    [~, optimize] = fminunc(@target_2,zeros(m,1));
    ret = optimize;
    output.ret = ret;
    output.y = y;
end

