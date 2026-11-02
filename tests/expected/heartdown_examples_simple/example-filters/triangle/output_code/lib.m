function output = filters()
% output = filters()
%
%    f(x) = { 1-|x| if |x| < 1
%             0 otherwise where x ∈ ℝ
%    
    function [ret] = f(x)
        assert(numel(x) == 1);

        if abs(x) < 1
            ret = 1 - abs(x);
        else
            ret = 0;
        end
    end

    output.f = @f;
end

