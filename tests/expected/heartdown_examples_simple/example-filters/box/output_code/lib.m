function output = filters()
% output = filters()
%
%    f(x) = { 1 if |x| < 1
%             0 otherwise where x ∈ ℝ
    function ret = f(x)
        ret_1 = 0;
        if abs(x) < 1
            ret_1 = 1;
        else
            ret_1 = 0;
        end
        ret = ret_1;
    end
end

