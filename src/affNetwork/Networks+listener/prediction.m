% Copyright: (C) 2012-2016 POETICON++, European Commission FP7 project ICT-288382
% CopyPolicy: Released under the terms of the GNU GPL v2.0.
% Copyright: (C) 2015 VisLab, Institute for Systems and Robotics,
%                Instituto Superior Técnico, Universidade de Lisboa, Lisbon, Portugal
% Author: Pedro Vicente, Atabak Dehban, Afonso Gonçalves, João Abrantes
% CopyPolicy: Released under the terms of the GNU GPL v2.0

%% Effect prediction, queries server
% Input:
%    Bottle1x11 vector with prior - Just doubles
%    5 desc for the tool
%    5 desc for the object
%    1 action
%
% Output:
%   5x5 matrix of probabilities distribution
%   p(EffectX, EffectY | nodeName1=nodeValue1, ... , nodeName12=nodeValue12)

clear;

%% Initialize BN
% initPmtk3;

%% Choice of network:
%bn = 'pca4sep'; % 2015

bn = 'pca_2016_2n_2c_2v'; % 2016
% bn = 'pca_2016_2n_2c_4v'; % 2016

%pca_2016_2n_2c_2v

switch bn
    
    %% 2015
    %%
    case 'pca4merge' % 2015
        load('../oldStuff/pcaNet-4mergeComp.mat');
    case 'pca6merge' % 2015
        load('../oldStuff/pca6mergecomp.mat');
    case 'pca4sep' % 2015
        load('../oldStuff/pca_2n_2C_noise_25b.mat');
        %load('pca_2n_2C_noise_2b.mat');
        
    %% 2016    
    case 'pca_2016_2n_2c_2v'
        load('pca_2016_2n_3c_1v.mat'); % 2016
    case 'pca_2016_2n_2c_4v'
        load('pca_2016_2n_2C_4v.mat'); % 2016
    otherwise
        error([bn ' is not a known Network']);
        
end

%% FIG SIZE AND POS

figWidth = 400;  % 450
figHeight = 380; % 430
figLeftDefault = 1900; %2000
figBottomDefault = 0; %50

figIndexX_MAX = 2;
figIndexY_MAX = 1;

figLeft = figLeftDefault;
figBottom = figBottomDefault;

%% MISC

queryActionID = 1;
queryActionName = 'draw';

%% YARP
% Initialize YARP:
LoadYarp;
import yarp.Port
import yarp.Bottle
done = 0;
query     = Bottle;
prior     = Bottle;
posterior = Bottle;
answer    = Bottle;
% Creating ports:
portInput  = Port;
portOutput = Port;

% First close the port just in case:
% (this is to try to prevent matlab from being unresponsive)
portInput.close;
portOutput.close;

% Open the ports:
disp('Opening ports...');
portInput.open('/Eprediction/read:i');
disp('opened port /Eprediction/read:i');
pause(0.5);
portOutput.open('/Eprediction/write:o');
disp('opened port /Eprediction/write:o');
pause(0.5);
disp('Done opening ports');
i=1;
display=true;
sameFig = false;
%% Run until you get the quit signal:
while(~done)
    
    query.clear()
    portInput.read( query );
    if (sum(size(query)) ~= 0)
        disp('Received command: ');
        disp(query);
        
        %checking for quit signal
        if(strcmp(query.toString, 'displayON'))
            display=true;
            disp('Display is now ON');
            continue;
        end
        if(strcmp(query.toString, 'displayOFF'))
            display=false;
            disp('Display is now OFF');
            continue;      
        end
        if(strcmp(query.toString, 'sameOFF'))
            sameFig=false;
            disp('Same Figure Display is now OFF');
            continue;      
        end
        if(strcmp(query.toString, 'sameON'))
            sameFig=true;
            disp('Same Figure Display is now OFF');
            continue;      
        end
        if (strcmp(query.toString, 'quit'))
            done=1;
            disp('Quiting');
            
        else    
            %prior.clear();
            %posterior.clear();
            disp('Calculating probabilities...');
            
            % Get the query priors and posteriors:
            prior = query;
            %posterior = query.get(1);
            %% Test if yarp bottle is with a good structure/size
            if(query.size()~=11)
                answer.fromString('nack')
                portOutput.write(answer);
                disp('Warning: Wrong Input size')
                continue;
            end
            %%
            switch bn
               case 'pca4merge'
                   prior_nodes  = [1 2 3 4 5]; % pca1 pca2 pca3 pca4 action
                    %prior_values = zeros(1, size(prior_nodes,2));
                    for n = 1:10 % features of the tool and object
                        features(n) = prior.get(n-1).asDouble;
                    end
                    %action = prior.get(0).asList().get(10).asDouble;
                    action = prior.get(10).asDouble;
                    score =  discretize(features*pinv(pc(:,1:components)'), ranges); % convert 
                                    % to pca - pc 10x10 matrix, discretize values
                    prior_values = [score action]; % and add the action to prior
                    posterior_nodes = [6 7]; % X and Y effect 
               case 'pca6merge'
                   prior_nodes  = [1 2 3 4 5 6 7]; % pca1 pca2 pca3 pca4 action
                    %prior_values = zeros(1, size(prior_nodes,2));
                    for n = 1:10 % features of the tool and object
                        features(n) = prior.get(n-1).asDouble;
                    end
                    %action = prior.get(0).asList().get(10).asDouble;
                    action = prior.get(10).asDouble;
                    score =  discretize(features*pinv(pc(:,1:components)'), ranges); % convert 
                                    % to pca - pc 10x10 matrix, discretize values
                    prior_values = [score action]; % and add the action to prior
                    posterior_nodes = [8 9]; % X and Y effect   
                case {'pca_2016_2n_2c_2v', 'pca_2016_2n_2c_4v', 'pca4sep'} 
                    prior_nodes  = [1 2 3 4 5]; % pca1_T pca2_T pca3_O pca4_O action
                    %prior_values = zeros(1, size(prior_nodes,2));
                    for n = 1:10 % features of the tool and object
                        features(n) = prior.get(n-1).asDouble;
                    end
                    %action = prior.get(0).asList().get(10).asDouble;
                    action = prior.get(10).asDouble;
                    featuresT = features(1:5);
                    featuresO = features(6:10);
                    score=[ featuresT*pinv(pcT(:,1:components)') featuresO*pinv(pcO(:,1:components)')];
                    score = discretize(score,ranges);
                    prior_values = [score action]; % and add the action to prior
                    posterior_nodes = [6 7]; % X and Y effect 
                case {'pca2n_3C', 'pca6sep'} 
                    prior_nodes  = [1 2 3 4 5 6 7]; % pca1_T pca2_T pca3_O pca4_O action
                    %prior_values = zeros(1, size(prior_nodes,2));
                    for n = 1:10 % features of the tool and object
                        features(n) = prior.get(n-1).asDouble;
                    end
                    %action = prior.get(0).asList().get(10).asDouble;
                    action = prior.get(10).asDouble;
                    featuresT = features(1:5);
                    featuresO = features(6:10);
                    score=[ featuresT*pinv(pcT(:,1:components)') featuresO*pinv(pcO(:,1:components)')];
                    score = discretize(score,ranges);
                    prior_values = [score action]; % and add the action to prior
                    posterior_nodes = [8 9]; % X and Y effect
                    
                otherwise
                    error([bn ' is not a known Network']);
            end

            
            % Set the priors and do the query p(posteriors|priors):
            clamped = sparsevec(prior_nodes, prior_values, BN.nnodes);
            prob = dgmInferQuery(BN, posterior_nodes, 'clamped', clamped, ...
                'doSlice', false);
            
            answer.clear;
            answer_string = '';
            answer_string=[answer_string '('];
            for k=1:size(prob.T,1)
                answer_string=[answer_string '('];
                for j=1:size(prob.T,1)
                     answer_string=[answer_string ' ' num2str(prob.T(k,j))];
                end
               answer_string=[answer_string ') '];
            end    
            answer_string=[answer_string ')'];
            %answer_string(end) = '';
            answer.fromString(answer_string);
                    
            x = [0.5,1.5,2.5,3.5,4.5];
            
            
            figIndexX = mod ((i-1),figIndexX_MAX);
            %figIndexX
            
            figIndexY = mod ( int32(fix((i-1)/figIndexX_MAX)) ,figIndexY_MAX);
            %figIndexY
            
            %figLeft = figLeftDefault + (i-1)*figWidth;
            figLeft = figLeftDefault + figIndexX*figWidth;
            %figLeft
            figBottom = figBottomDefault + figIndexY*figHeight;
            %figBottom
            
            if (action == 1.0)
                queryActionName = 'draw';
            elseif (action == 2.0)
                queryActionName = 'push';
            end
            if(display)
                if(sameFig)
                    hFig(i)=figure(1);
                else
                    hFig(i) = figure(i);
                end
                str = sprintf('%s prediction %d',queryActionName, i); % maybe change the title :p
                title(str);
                set(hFig(i), 'Position', [figLeft figBottom figWidth figHeight]) % maybe change the position of the window
                axis([0 5 0 5.5])
                hold on;
                scatter (x, 0.5*ones(1,5) , 5000, prob.T(1,:),'filled','s')
                hold on;
                scatter (x, 1.5*ones(1,5) , 5000, prob.T(2,:),'filled','s')
                hold on;
                scatter (x, 2.5*ones(1,5) , 5000, prob.T(3,:),'filled','s')
                hold on;
                scatter (x, 3.5*ones(1,5) , 5000, prob.T(4,:), 'filled','s')
                hold on;
                scatter (x, 4.5*ones(1,5) , 5000, prob.T(5,:),'filled','s')
                hold on;
                colormap gray;
                plot(2.5,5.25,'r*','LineWidth',8) ; % Display robot position
                pause(1);
                i=i+1;
            end
            portOutput.write(answer);
            disp('Done');            
        end
    end
    pause(0.01);
end
disp('Going to close the ports');
portInput.close;
portOutput.close;
close all;
clear;
