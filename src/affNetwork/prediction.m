% Copyright: (C) 2012-2015 POETICON++, European Commission FP7 project ICT-288382
% CopyPolicy: Released under the terms of the GNU GPL v2.0.
% Copyright: (C) 2015 VisLab, Institute for Systems and Robotics,
%                Instituto Superior Técnico, Universidade de Lisboa, Lisbon, Portugal
% Author: Afonso Gonçalves, Pedro Vicente, Giovanni Saponaro
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
bn = 'pca6merge';

switch bn
    case 'pca4merge'
        load('pcaNet-4mergeComp.mat');
    case 'pca6merge'
		load('pca6mergecomp.mat');
	case 'pca4sep'
		load('pca2T-2O.mat');
    otherwise
        error([bn ' is not a known Network']);
        
end

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

%% Run until you get the quit signal:
while(~done)
    
    query.clear()
    portInput.read( query );
    if (sum(size(query)) ~= 0)
        disp('Received command: ');
        disp(query);
        
        %checking for quit signal
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
                case 'pca4sep'
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
            portOutput.write(answer);        

            disp('Done');            
        end
    end
    pause(0.01);
end
disp('Going to close the ports');
portInput.close;
portOutput.close;
clear;
