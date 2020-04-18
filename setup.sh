#!/usr/bin/env bash

if [[ ! -d "data" ]]; then
	mkdir data
fi

# download example data
if [[ ! -d "data/3RScan" ]]; then
	if [[ ! -f "data/3RScan.zip" ]]; then
		wget "http://campar.in.tum.de/files/3RScan/3RScan.zip" -P data
	fi
	tar -xvf "data/3RScan.zip" -C data
fi

# download 3RScan metafile
if [[ ! -f "data/3RScan/3RScan.json" ]]; then
	wget "http://campar.in.tum.de/files/3RScan/3RScan.json" -P data/3RScan
fi
if [[ ! -f "data/3RScan/objects.json" ]]; then
	wget "http://campar.in.tum.de/files/3DSSG/3DSSG/objects.json" -P data/3RScan
fi
if [[ ! -f "data/3RScan/relationships.json" ]]; then
	wget "http://campar.in.tum.de/files/3DSSG/3DSSG/relationships.json" -P data/3RScan
fi

echo "sucessfully setup data folder."