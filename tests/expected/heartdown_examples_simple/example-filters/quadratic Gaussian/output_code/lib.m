function output = filters()
% output = filters()
%
%    f(x) = { 1-3x²               if |x| < ⅓
%             1.5x² - 3|x| + 1.5  if |x| < 1
%             0 otherwise where x ∈ ℝ
    function ret = f(x)
        ret_1 = 0;
        if abs(x) < (1/3)
            ret_1 = 1 - (3 * (x.^2));
        elseif abs(x) < 1
            ret_1 = 1.5 * (x.^2) - (3 * abs(x)) + 1.5;
        else
            ret_1 = 0;
        end
        ret = ret_1;
    end
end

