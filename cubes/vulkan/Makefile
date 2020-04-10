VULKAN_SDK_PATH = $(VULKAN_SDK)
STB_INCLUDE_PATH = /usr/local/include/

IDIR = include
IDIR_EXT = ../include
ODIR = obj
ODIR_EXT = ../obj
SDIR = src
SDIR_EXT = ../src

CC=g++
CFLAGS = -std=c++17 -I$(VULKAN_SDK_PATH)/include -I$(STB_INCLUDE_PATH) -I$(IDIR) -I$(IDIR_EXT)
LDFLAGS = -L$(VULKAN_SDK_PATH)/lib `pkg-config --static --libs glfw3` -lvulkan

_DEPS = evulkan.h
_DEPS_EXT = cube.h grid.h vertex.h util.h 
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))
DEPS += $(patsubst %,$(IDIR_EXT)/%,$(_DEPS_EXT))

_OBJ =  descriptor.o instance.o render.o run.o swap.o buffer.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

_OBJ_EXT = util.o
OBJ_EXT = $(patsubst %,$(ODIR_EXT)/%,$(_OBJ_EXT))

.PHONY: test clean

$(ODIR)/%.o: $(SDIR)/%.cpp $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

$(ODIR_EXT)/%.o: $(SDIR_EXT)/%.cpp $(DEPS_EXT)
	$(CC) $(CFLAGS) -c -o $@ $<

libEVulkan.a: $(OBJ) $(OBJ_EXT)
	ar rcs $@ $^

libs: libEVulkan.a

clean:
	rm vulkan || true
	rm $(ODIR)/*.o || true
	rm libEVulkan.a || true
