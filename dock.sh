#!/bin/bash

# links that might help w/ macOS + docker visuals:
#
# https://gist.github.com/cschiewek/246a244ba23da8b9f0e7b11a68bf3285
# https://stackoverflow.com/questions/72586838/xquartz-cant-open-display-mac-os

image=team17/cs205_ubuntu:latest
dockerdir=$PWD/.container
container=team17
username=team17
workdir=/team17

# change to 'true' if you're on MacOS
using_mac_os=false

# files to copy into the container
src='
	src/Makefile
	src/hellobox2d.cpp
	src/helloopengl.cpp
	src/main.cpp
	src/walker.cpp
	src/walker_state.cpp
	src/walker_parameters.cpp
	src/render
	src/CMakeLists.txt
	src/trajectory.cpp
	src/err.cpp
'
include='
	src/include/statics.h
	src/include/walker.h
'

clean() {
	echo "Removing container $container ..."
	sudo docker rm -f $container

	echo "Removing image $image ..."
	sudo docker rmi $image

	echo "Removing docker copy directory $dockerdir ..."
	rm -rf $dockerdir

	echo "Done cleaning."
}

# USAGE: ./dock.sh clean
# delete all files/directories created by this script and exit
if [ $# -gt 0 ] && [ $1 == "clean" ];
then
	clean
	exit 0
fi

# USAGE: ./dock.sh fresh
# delete all files, images, containers created by this script prior to running
if [ $# -gt 0 ] && [ $1 == "fresh" ];
then
	clean
fi

# ensure the file exchange directory exists
if [ ! -d $dockerdir ];
then
	echo "Docker copy directory does not exist. Creating $dockerdir ..."
	mkdir $dockerdir $dockerdir/src $dockerdir/include
fi

# copy relevant files for the running container to access
cp $src $dockerdir/src
cp $include $dockerdir/include

# build the container image if it doesn't exist
if [ "$(sudo docker image inspect $image 2> /dev/null)" == "[]" ]; 
then
	echo "Container image does not exist. Building $image ..."
	sudo docker build -t $image .
else
	echo "Container image already exists. Skipping the building process."
fi

echo "Running container $container ..."

echo "[macOS]: note that 'using_mac_os' is set to $using_mac_os"

# run the container with X11 forwarding
if [ "$using_mac_os" = true ];
then
	echo 	"[macOS] make sure that XQuartz is running and is allowing " \
			"connections from network clients; see the link(s) commented in " \
			"this script for more information"

	# [WARNING] apparently using `xhost` is insecure; the Docker container has
	# permissions limited to help w/ this but honestly idk, turn off XQuartz
	# as soon as it's convenient
	xhost + "${IP}"

	sudo docker run -d -i -t \
				--name $container \
				-e DISPLAY="${IP}:0" \
				--cap-drop=ALL \
				--cap-add=CAP_CHOWN \
				$image
else
	sudo docker run -d -i -t \
					--name $container \
					-e DISPLAY=$DISPLAY \
					--volume=/tmp/.X11-unix:/tmp/.X11-unix \
					--cap-drop=ALL \
					--cap-add=CAP_CHOWN \
					$image
fi

# give the (non-root) container user ownership of all container files
sudo docker cp $dockerdir/src/. $container:$workdir
sudo docker cp $dockerdir/include/. $container:$workdir/include
sudo docker exec -d $container chown -R $username $workdir

# access the container via bash
sudo docker exec -it -w $workdir -u $username $container /bin/bash