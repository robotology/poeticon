% Copyright: (C) 2012-2016 POETICON++, European Commission FP7 project ICT-288382
% CopyPolicy: Released under the terms of the GNU GPL v2.0.
% Copyright: (C) 2015 VisLab, Institute for Systems and Robotics,
%                Instituto Superior Técnico, Universidade de Lisboa, Lisbon, Portugal
% Author: Afonso Gonçalves, Pedro Vicente, Giovanni Saponaro
% CopyPolicy: Released under the terms of the GNU GPL v2.0


function [ output_args ] = combinator( fileDesc, fileEff, fileAff )
%[ output_args ] = combinator( fileDesc, fileEff, fileAff )
%UNTITLED Summary of this function goes here
%   Detailed explanation goes here
fid_desc = fopen(fileDesc,'r');
fid_eff  = fopen(fileEff, 'r');
%fid_aff  = fopen(fileAff, 'w');
dlmwrite(fileAff, [], 'delimiter','');

desc = textscan(fid_desc,'%s %d %f %f %f %f %f %f %f %f %f %f'); % desc toolid 5blobDesc 5effDesc
eff  = textscan(fid_eff, '%s %d %d %d %f %f %f %d %d %f %f %f %d %d %f %f %f %d %d %f %f %f %d %d %f %f %f %d %d');

for n = 1:size(eff{1,1},1)
    
    indTool   = find(desc{1,2}==eff{1,2}(n));
    indTarget = find(desc{1,2}==eff{1,3}(n));
    
    temp_aff(1:(size(indTool,1)*size(indTarget,1)),1:13) = NaN;
    
    for Tool = 1:size(indTool,1)
        for Target = 1:size(indTarget,1)
            for d = 1:5 % tool descriptor
                temp_aff(Target+(Tool-1)*size(indTarget,1),d) = desc{1,d+7}(indTool(Tool));
            end
            for d = 1:5
                temp_aff(Target+(Tool-1)*size(indTarget,1),d+5) = desc{1,d+3}(indTarget(Target));
            end
            temp_aff(Target+(Tool-1)*size(indTarget,1),11) = eff{1,4}(n);% action
            temp_aff(Target+(Tool-1)*size(indTarget,1),12) = eff{1,25}(n) - eff{1,5}(n); %effectX
            temp_aff(Target+(Tool-1)*size(indTarget,1),13) = eff{1,26}(n) - eff{1,6}(n); %effectY
            
        end     
    end  
    dlmwrite(fileAff, temp_aff, 'delimiter',' ','-append');
    clear temp_aff
end

end

