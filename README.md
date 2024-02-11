# Teaching a Walking Agent with Genetic Algorithms

## contents

- `deliverables` : project milestone submission files
- `code` : source code + scripts
- `src/retired` : obselete scripts
- `Dockerfile` : dockerfile to create a container the project can fully run within
- `dock.sh` : script to build/enter the docker container

## docker

- this project has a Dockerfile so we can all have the same development environment
- all of the Docker stuff was scripted/tested on WSL2 Ubuntu; I *think* it'll work for anything Unix-based (i.e. macOS too)
- the docker image is just a small addition to the class docker image `iacs/cs205_ubuntu`, so if that works for you, this *probably* will as well

## how to run simulations + visualizations

1. enter the Docker container by running `dock.sh fresh`
    - the `fresh` argument deletes the current Docker image and container, if they exist; this allows changes to code to be seen in docker
    - running without the `fresh` argument will just run the same container last used, which will not reflect any changes in code
    - running `dock.sh clean` just removes the old image/container and exits
2. wait a second...
3. `make main` to generate the `main` executable
    - any edits to `include/statics.h` or `main.cpp` to change simulation parameters should be done now
    - some simulation parameters can be changed without `make`-ing `main` again
4. run `main` with the appropriate simulation parameters 
    - `main` currently accepts 3 command-line arguments; if not specified the defaults from `include/statics.h` are used
    - i.e. `main {number of walker} {number of iterations/generations} {fittest ratio}`
    - ex. `./main 500 1000 0.01` to simulation 1000 generations, each with 500 walkers, using the top 1% of walkers to produce the next generation
5. run the `render` script to visualize the simulation in the Box2d "Testbed"
    - running `main` produces a `trajectory.json` file that the visualization tries to replicate
    - approximate error between the original simulation and the visualization is given in the command-line
    - [TODO] error can be improved, but it would take a bit of work
  
This is a joint project by Omar Abdel Haq, Elie Eshoa, Ricky Williams, and May Soshi.

## how to run `hellobox2d` demo (w/ Docker)

to run the basic Box2D demo `hellobox2d.cpp`
1. run `dock.sh` to enter the Docker container for the project 
2. run `make hellobox2d` to generate the `hellobox2d` executable
3. run the `hellobox2d` executable
