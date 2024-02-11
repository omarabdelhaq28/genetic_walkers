#!/bin/bash

# TEMPORARY script for visualizing "example_walker.cpp" within the testbed
# this should be replaced with script(s) for "automatic" visualization of
# simulation code + loading visualizations into testbed + running testbed
#
# note that running the testbed requires editing the CMake file
# box2d/testbed/CMakeLists.txt, which was manually done on my machine locally;
# even more of a reason to replace this script with more portable methods
# (e.g. visualization wrapper class for simulations + updated Dockerfile
# for minimal testbed building)

cp example_walker.cpp box2d/testbed/tests
cd box2d
./build.sh
./build/bin/testbed
cd ..