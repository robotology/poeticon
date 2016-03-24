Affordances Network
==========

## Data+combinator

In this folder one can find the data used to trained the Bayesian Network. 

For more detail on data acquisition see:

- [affExploreManager] (https://github.com/robotology/poeticon/tree/master/src/affExploreManager)
- [3d_pull_push_data] (https://github.com/atabakd/3d_pull_push_data)

## Training_Network
The files: pcaTrain_2n2c_2v and pcaTrain_2n2c_4v are the most important ones.

- n: number of PCA. 
- c: number of components for each PCA
- v: number of discretization values/bins of each PCA component

pcaTrain_2n2c_2v loads the data from Data+combinator folder (change the path if you want to use a different data - line 15)

The script outputs the trained Network (with the default name: line 117) to the Networks+ĺistener folder

## Networks+ĺistener

This folder contains the trained Networks and the listener matlab script.

The listener (prediction.m) loads one of the Networks, bn variable (the default value is bn = 'pca_2016_2n_2c_2v') and waits for queries. 

See prediction.m for a more detailed information.

## oldStuff

Old Networks and scripts used in the 3rd year demo - Backup

## License

Released under the terms of the GPL v2.0 or later. See the file LICENSE for details.

