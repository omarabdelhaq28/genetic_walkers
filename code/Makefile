CXX 		:= g++
CXXFLAGS 	:= -Wall -std=c++11 -flto -Iinclude/ -Llib/ -fopenmp
LDFLAGS_B2	:= -lbox2d
LDFLAGS_GL 	:= -lGL -lglut

SOURCES		:= main.cpp hellobox2d.cpp helloopengl.cpp err.cpp
CLASSES		:= walker.cpp walker_state.cpp walker_parameters.cpp
HEADER		:= include/statics.h include/walker.h

OBJS		:= $(patsubst %.cpp,%.o,$(SOURCES) $(CLASSES))
BINS		:= $(patsubst %.cpp,%,$(SOURCES))

# linked libraries set per target
all: lib = $(LDFLAGS_B2) $(LDFLAGS_GL)
main: lib = $(LDFLAGS_B2)
hellobox2d: lib = $(LDFLAGS_B2)
helloopengl: lib = $(LDFLAGS_GL)
err: lib = $(LDFLAGS_B2)

all: $(BINS)

$(OBJS): %.o: %.cpp

%: %.o
	$(CXX) $(CXXFLAGS) $^ $(lib) -o $@

main: main.o walker.o walker_state.o walker_parameters.o $(HEADER)
err: err.o walker.o walker_state.o walker_parameters.o $(HEADER)

clean:
	rm -rf $(BINS) $(OBJS) *.json