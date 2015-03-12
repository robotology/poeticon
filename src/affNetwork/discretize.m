function data = discretize(data,ranges)
%usa os ranges conseguidos em getRanges.m para discretizar os dados
%exemplo 
% data = (1:10)';
% result = discretize(data,getRanges([2],data));
% result => 1111122222
components = size(data,2);
for i = 1:components
    [~,bin]=histc(data(:,i),ranges{i});
    data(:,i)=bin+1;
end