% Copyright: (C) 2012-2016 POETICON++, European Commission FP7 project ICT-288382
% CopyPolicy: Released under the terms of the GNU GPL v2.0.
% Copyright: (C) 2015 VisLab, Institute for Systems and Robotics,
%                Instituto Superior Técnico, Universidade de Lisboa, Lisbon, Portugal
% Author: Afonso Gonçalves, João Abrantes, Pedro Vicente, Giovanni Saponaro
% CopyPolicy: Released under the terms of the GNU GPL v2.0

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
