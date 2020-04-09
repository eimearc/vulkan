VULKAN_SDK_PATH = $(VULKAN_SDK)
STB_INCLUDE_PATH = /usr/local/include/

IDIR = include
ODIR = obj
ODIR_DEPS = ../obj
SDIR = src
SDIR_DEPS = ../src

CC=g++
CFLAGS = -std=c++17 -I$(VULKAN_SDK_PATH)/include -I$(STB_INCLUDE_PATH) -I$(IDIR) -I../include
LDFLAGS = -L$(VULKAN_SDK_PATH)/lib `pkg-config --static --libs glfw3` -lvulkan

_DEPS = evulkan.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))
DEPS += ../include/cube.h ../include/grid.h ../include/vertex.h ../include/util.h

_OBJ =  main.o descriptor.o instance.o render.o run.o swap.o buffer.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

_OBJ_DEPS = util.o
OBJ_DEPS = $(patsubst %,$(ODIR_DEPS)/%,$(_OBJ_DEPS))

vulkan: $(OBJ) $(OBJ_DEPS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

.PHONY: test clean

$(ODIR)/%.o: $(SDIR)/%.cpp $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

$(ODIR_DEPS)/%.o: $(SDIR_DEPS)/%.cpp $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

test: Cubes
	LD_LIBRARY_PATH=$(VULKAN_SDK_PATH)/lib VK_LAYER_PATH=$(VULKAN_SDK_PATH)/etc/vulkan/explicit_layer.d ./vulkan

clean:
	rm -f vulkan
	rm -f $(ODIR)/*.o
