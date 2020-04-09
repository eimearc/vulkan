STB_INCLUDE_PATH = /usr/local/include/

IDIR = include
IDIR_EXT = ../include
ODIR = obj
ODIR_EXT = ../obj
SDIR = src
SDIR_EXT = ../src

CC=g++
CFLAGS = -std=c++17 -I$(STB_INCLUDE_PATH) -I$(IDIR) -I$(IDIR_EXT)
GLEW_PATH = /usr/local/lib/
LDFLAGS = `pkg-config --static --libs glfw3` -L$(GLEW_PATH) -lGLEW -framework OpenGL

_DEPS = egl.h
_DEPS_EXT = cube.h grid.h vertex.h util.h 
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))
DEPS += $(patsubst %,$(IDIR_EXT)/%,$(_DEPS_EXT))

_OBJ = main.o egl.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

_OBJ_EXT = util.o
OBJ_EXT = $(patsubst %,$(ODIR_EXT)/%,$(_OBJ_EXT))

gl: $(OBJ) $(OBJ_EXT)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

.PHONY: test clean

$(ODIR)/%.o: $(SDIR)/%.cpp $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

$(ODIR_EXT)/%.o: $(SDIR_EXT)/%.cpp $(DEPS_EXT)
	$(CC) $(CFLAGS) -c -o $@ $<

test: gl
	./gl

clean:
	rm -f gl
	rm -f $(ODIR)/*.o
