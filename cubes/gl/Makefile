STB_INCLUDE_PATH = /usr/local/include/

IDIR = include
ODIR = obj
SDIR = src

CC=g++
CFLAGS = -std=c++17 -I$(STB_INCLUDE_PATH) -I$(IDIR) -I../include/
GLEW_PATH = /usr/local/lib/
LDFLAGS = `pkg-config --static --libs glfw3` -L$(GLEW_PATH) -lGLEW -framework OpenGL

_DEPS = ../include/cube.h ../include/grid.h include/egl.h ../include.h

_OBJ = main.o egl.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

gl: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

.PHONY: test clean

$(ODIR)/%.o: $(SDIR)/%.cpp $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

test: gl
	./gl

clean:
	rm -f gl
	rm -f $(ODIR)/*.o
