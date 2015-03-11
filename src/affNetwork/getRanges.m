function ranges = getRanges(values,DATA)
%DATA each column is to be discretize in values[r] values.
%retorna os ranges que separam a coluna da DATA[:,c] em values[c] bins
%iguais.
%Exemplo ranges = getRanges([2],(1:10)') quero os numeros de 1 a 10
%separados em dois bins iguais
% ranges = {[5;10]}
% de menos infinito a 5 e' um grupo de 5 a 10 e' outro 
epsilon = 10^(-10);
components = length(values);
ranges = cell(components,1);%zeros(values-1,components);
for c=1:components
    ranges{c} = zeros(values(c),1);
end
for c = 1:components
    temp = DATA(:,c);
    F = ceil(values(c)*tiedrank(temp)/size(DATA,1));
    for t = 1:(values(c)) %upper threshold
        r = temp(F==t);
        if ~isempty(r)
            ranges{c}(t) = max(r)+epsilon;
        else
            fprintf('Cuidado com a coluna %d\n',c);
            ranges{c}(t) = max(temp)+t;
        end
    end
end
