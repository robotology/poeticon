function [Data] = discretizeAfonso(rawData,k)
% [Data] = discretizeAfonso(rawData,k)
% discretize the rawData - Afonso's way
% 
% rawData - value to discretize
%
% k - int index of the attribute:
% Tool Effector:
% 1- toolArea; 2- toolConv; 3- toolEcce; 
% 4- toolComp; 5- toolCirc; 6- toolSqua
% Object:
% 7- objectArea; 8- objectConv; 9- objectEcce; 
% 10- objectComp; 11- objectCirc; 12- objectSqua
%


    % Discretization of toolArea:
  switch(k)
        case 1
            if (rawData <=1000)
                Data = 1;
            elseif (1000 < rawData && rawData <=2000)
                Data = 2;
            elseif (2000 < rawData)
                Data = 3;
            end

            % Discretization of toolConv:
        case 2
            if (rawData <= 0.6)
                Data = 1;
            elseif (0.6 < rawData && rawData <= 0.7)
                Data = 2;
            elseif (0.7 < rawData)
                Data = 3;
            end

            % Discretization of toolEcce:
        case 3  
            if (rawData <= 0.45)
                Data = 1;
            elseif (0.45 < rawData && rawData <= 0.6)
                Data = 2;
            elseif (0.6  < rawData)
                Data = 3;
            end

            % Discretization of toolComp:
        case 4
            if (rawData <= 0.0175)
                Data = 1;
            elseif (0.0175 < rawData && rawData <= 0.025)
                Data = 2;
            elseif (0.025 < rawData)
                Data = 3;
            end

            % Discretization of toolCirc:
        case 5
            if (rawData <= 0.22)
                Data = 1;
            elseif (0.22 < rawData && rawData <= 0.35)
                Data = 2;
            elseif (0.35 < rawData)
                Data = 3;
            end

            % Discretization of toolSqua:
        case 6
            if(rawData <= 0.44)
                Data = 1;
            elseif (0.44 < rawData && rawData <= 0.6)
                Data = 2;
            elseif (0.6 < rawData)
                Data = 3;
            end

            % Discretization of objectArea:
        case 7
            if (rawData <= 1000)
                Data = 1;
            elseif (1000 < rawData && rawData <= 1800)
                Data = 2;
            elseif (1800 < rawData)
                Data = 3;
            end

            % Discretization of objectConv:
        case 8
            if (rawData <= 0.92)
                Data = 1;
            elseif (0.92 < rawData && rawData <= 0.96)
                Data = 2;
            elseif (0.96 < rawData)
                Data = 3;
            end

            % Discretization of objectEcce:
        case 9
            if (rawData <= 0.37)
                Data = 1;
            elseif (0.37 < rawData && rawData <= 0.66)
                Data = 2;
            elseif (0.66 < rawData)
                Data = 3;
            end

            % Discretization of objectComp:
        case 10
            if (rawData <= 0.021)
                Data = 1;
            elseif (0.021 < rawData && rawData <= 0.05)
                Data = 2;
            elseif (0.05 < rawData)
                Data = 3;
            end

            % Discretization of objectCirc:
        case 11
            if (rawData <= 0.4)
                Data = 1;
            elseif (0.4 < rawData && rawData <= 0.65)
                Data = 2;
            elseif (0.65 < rawData)
                Data = 3;
            end

            % Discretization of objectSqua:
        case 12
            if  (rawData <= 0.48)
                Data = 1;
            elseif (0.48 < rawData && rawData <= 0.77)
                Data = 2;
            elseif (0.77 < rawData)
                Data = 3;
            end
      otherwise
           Data = rawData;
  end
end