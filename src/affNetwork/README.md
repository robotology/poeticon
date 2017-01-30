Affordances Network
==========

## Data+combinator

In this folder you can find the data used to train our affordances Bayesian Networks.

For more detail on affordance datasets and on the data acquisition process with the iCub, see also:

- [affExploreManager] (https://github.com/robotology/poeticon/tree/master/src/affExploreManager)
- [3d_pull_push_data] (https://github.com/atabakd/3d_pull_push_data)
- [A Dataset on Visual Affordances of Objects and Tools] by A. Dehban (http://vislab.isr.tecnico.ulisboa.pt/datasets/)

## Training_Network

The files `pcaTrain_2n2c_2v.m` and `pcaTrain_2n2c_4v.m` are the most important ones, where

- n: number of PCA;
- c: number of components for each PCA;
- v: number of discretization values/bins of each PCA component.

`pcaTrain_2n2c_2v.m` loads data from `Data+combinator` folder (change the path in line 15 if you want to use a different data).

The script outputs the trained network as a `.mat` file in the `Networks+listener` folder (you may change the filename in line 117).

## Networks+Listener

This folder contains the trained networks and the listener MATLAB YARP script.

The listener (`prediction.m`) loads one of the networks according to the `bn` variable (the default value being `pca_2016_2n_2c_2v`) and waits for queries.

See `prediction.m` for more detailed information.

## oldStuff

Old networks and scripts that were used in the 3rd year demo (March 2015) are kept here for historical reference.

## License

Released under the terms of the GPL v2.0 or later. See `LICENSE` for details.
