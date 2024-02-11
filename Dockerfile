FROM iacs/cs205_ubuntu

# install dependencies (X11, OpenGL)
RUN apt-get -y update
RUN apt-get -y upgrade
RUN apt-get -y install xorg-dev freeglut3-dev

# additional dependencies needed for some OpenGL issue(?)
# see https://bugs.launchpad.net/cloud-images/+bug/2007555 
RUN yes | apt-get install -y --allow-downgrades \
    libglapi-mesa=22.0.1-1ubuntu2 \
    libgbm1=22.0.1-1ubuntu2 \
    libegl-mesa0=22.0.1-1ubuntu2 \
    libgl1-mesa-dri=22.0.1-1ubuntu2 \
    libglx-mesa0=22.0.1-1ubuntu2

# download and build Box2D
RUN mkdir /team17
WORKDIR /team17
RUN git clone https://github.com/erincatto/box2d.git
WORKDIR /team17/box2d
RUN ./build.sh
RUN cp -r ./build/bin /team17/lib
RUN cp -r ./include /team17/include

# dowmload Nlohmann JSON
WORKDIR /tmp
RUN git clone https://github.com/nlohmann/json.git
RUN cp -r json/single_include/nlohmann /team17/include

# the container will be run as user "team17"
RUN adduser team17

# cleanup
WORKDIR /team17