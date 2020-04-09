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

_OBJ =  main.o descriptor.o instance.o render.o run.o swap.o buffer.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

_OBJ_EXT = util.o
OBJ_EXT = $(patsubst %,$(ODIR_EXT)/%,$(_OBJ_EXT))

vulkan: $(OBJ) $(OBJ_EXT)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

.PHONY: test clean

$(ODIR)/%.o: $(SDIR)/%.cpp $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

$(ODIR_EXT)/%.o: $(SDIR_EXT)/%.cpp $(DEPS_EXT)
	$(CC) $(CFLAGS) -c -o $@ $<

test: vulkan
	LD_LIBRARY_PATH=$(VULKAN_SDK_PATH)/lib VK_LAYER_PATH=$(VULKAN_SDK_PATH)/etc/vulkan/explicit_layer.d ./vulkan

clean:
	rm -f vulkan
	rm -f $(ODIR)/*.o
