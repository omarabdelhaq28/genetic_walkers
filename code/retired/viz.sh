#!/bin/bash

# viz locally (not on docker)

if [ ! -d "box2d" ]; then
    # git clone https://github.com/erincatto/box2d.git
    git clone git@github.com:erincatto/box2d.git
fi

cp CMakeLists.txt box2d/testbed
cp -r include/walker.h include/statics.h include/nlohmann box2d/testbed
cp walker.cpp box2d/testbed
cp trajectory.cpp box2d/testbed/tests
cd box2d
./build.sh
./build/bin/testbed
cd ..