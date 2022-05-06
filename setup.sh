#!/usr/bin/env bash

if [[ ! -d "data" ]]; then
	mkdir data
fi

# download example data
if [[ ! -d "data/3RScan" ]]; then
	if [[ ! -f "data/3RScan.v2.zip" ]]; then
		wget "http://campar.in.tum.de/public_datasets/3RScan/3RScan.v2.zip" -P data
	fi
	unzip "data/3RScan.v2.zip" -d ./data/3RScan
fi

# download 3RScan metafile and 3DSSG data
if [[ ! -f "data/3RScan/3RScan.json" ]]; then
	wget "http://campar.in.tum.de/public_datasets/3RScan/3RScan.json" -P data/3RScan
fi
if [[ ! -f "data/3RScan/objects.json" ]]; then
	wget "http://campar.in.tum.de/public_datasets/3DSSG/3DSSG/objects.json" -P data/3RScan
fi
if [[ ! -f "data/3RScan/relationships.json" ]]; then
	wget "http://campar.in.tum.de/public_datasets/3DSSG/3DSSG/relationships.json" -P data/3RScan
fi

echo "sucessfully setup data folder."
