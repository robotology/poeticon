% Copyright: (C) 2012-2015 POETICON++, European Commission FP7 project ICT-288382
% CopyPolicy: Released under the terms of the GNU GPL v2.0.
% Copyright: (C) 2015 VisLab, Institute for Systems and Robotics,
%                Instituto Superior Técnico, Universidade de Lisboa, Lisbon, Portugal
% Author: Afonso Gonçalves, Pedro Vicente, Giovanni Saponaro
% CopyPolicy: Released under the terms of the GNU GPL v2.0

%% Effect prediction, queries server
% Input:
%    Bottle1x13 vector with prior
%    6 desc for the tool
%    6 desc for the object
%    1 action

% Output:
%   5x5 matrix of probabilities distribution
%   p(EffectX, EffectY | nodeName1=nodeValue1, ... , nodeName12=nodeValue12)

clear;

%% Initialize BN
% initPmtk3;

%% Choice of network:
bn = 'pca';

switch bn
    case 'exp'
        load('./original_bn.mat');
    case 'K2'
        load('./k2_bn.mat');
    case 'Bde'
        load('./BDe_bn.mat');
    case 'pca'
        load('./PCA2_bn.mat'); 
        load('./princomp.mat');
        components = 2;
    case 'pca2'
        load('pca2ndVersion.mat');
        components = 2;
    otherwise
        load('./original_bn.mat');
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
                case 'exp'
                    prior_nodes  = 1:13;
                    prior_values = zeros(1, size(prior_nodes, 2));
                    for n = 1:size(prior_nodes, 2)
                        prior_values(n) = discretizeAfonso(prior.get(n-1).asDouble(),n);
                    end
                    posterior_nodes = [14 15];
                    
                case 'K2'
                    prior_nodes  = [1 2 7 14];
                    trans_nodes = [13 1 6 11];
                    prior_values = zeros(1, size(prior_nodes, 2));
                    for n = 1:size(prior_values, 2)
                        prior_values(n) = ...
                            discretizeAfonso(prior.get(trans_nodes(n)-1).asDouble, trans_nodes(n));
                    end
                    posterior_nodes = [11 15];
                    
                case 'Bde'
                    prior_nodes  = [2 3 4 5 10 11 15];
                    trans_nodes  = [5 1 3 13 10 6 9];
                    prior_values = zeros(1, size(prior_nodes,2));
                    for n = 1:size(prior_values, 2)
                        prior_values(n) = ...
                        discretizeAfonso(prior.get(trans_nodes(n)-1).asDouble, trans_nodes(n));
                    end
                    posterior_nodes = [9 13];
                case 'pca'
                    
                    prior_nodes  = [1 2 3]; % pca1 pca2 action
                    %prior_values = zeros(1, size(prior_nodes,2));
                    for n = 1:12 % features of the tool and object
                        features(n) = prior.get(n-1).asDouble;
                    end
                    action = prior.get(12).asDouble;
                    score =  discretize(features*pinv(pc(:,1:components)'), ranges); % convert 
                                    % to pca - pc 12x12 matrix, discretize values
                    prior_values = [score action]; % and add the action to prior
                    posterior_nodes = [4 5]; % X and Y effect
                case 'pca2'
                   prior_nodes  = [1 2 3]; % pca1 pca2 action
                    %prior_values = zeros(1, size(prior_nodes,2));
                    for n = 1:10 % features of the tool and object
                        features(n) = prior.get(n-1).asDouble;
                    end
                    action = prior.get(0).asList().get(10).asDouble;
                    score =  discretize(features*pinv(pc(:,1:components)'), ranges); % convert 
                                    % to pca - pc 10x10 matrix, discretize values
                    prior_values = [score action]; % and add the action to prior
                    posterior_nodes = [4 5]; % X and Y effect 
                otherwise
                    prior_nodes  = 1:13;
                    prior_values = zeros(1,size(prior_nodes,2));
                    for n = 1:size(prior_nodes, 2)
                        prior_values(n) = discretizeAfonso(prior.get(0).asList().get(n-1).asDouble(),n);
                    end
                    posterior_nodes = [14 15];
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
%             B=reshape(prob.T,25,1);
%             IX=1:25;
%             
%             numberprob = 25;
% 
%             prob_value = zeros( numberprob, 1 );
%             if size(prob.T,2) == 1
%                 prob_arg = zeros( numberprob, 1 );
%             else
%                 prob_arg = zeros( numberprob, size(size(prob.T), 2) );
%             end
%             prob_arg(:,1) = IX(1:numberprob);
%             
%             % Get the top probabilities (numberOfProb is the number of top
%             % answers):
%             for topprob_number = 1:numberprob
%                 prob_value(topprob_number) = B(topprob_number);
%                 % Convert the linear index into multidimensional indexes
%                 % (the indexes are the arguments that maximize the
%                 % probability queried):
%                 if size(prob.T,2) > 1
%                     for parameter = 1:size(size(prob.T),2)-1
%                         [ prob_arg(topprob_number, parameter), ...
%                             prob_arg(topprob_number, parameter+1) ] = ...
%                             ind2sub(size(prob.T), ...
%                             prob_arg(topprob_number, parameter));
%                     end
%                 end
%             end
%             
%             % Reply the top answers and correspondent probabilities to the
%             % query:
%             answer.clear;
%             answer_string = '';
%             
%             for ans_n = 1:numberprob
%                 answer_string=[answer_string '('];
%                 answer_string=[answer_string num2str(prob_value(ans_n))];
%                 for parameter = 1:size(prob_arg,2)
%                     answer_string=[answer_string ' ' num2str(prob_arg(ans_n, parameter))];
%                 end
%                 answer_string=[answer_string ') '];
%             end
%             
%             answer_string(end) = '';
%             answer.fromString(answer_string);
           
            

            disp('Done');            
        end
    end
    pause(0.01);
end
disp('Going to close the ports');
portInput.close;
portOutput.close;
clear;