VULKAN_SDK_PATH = $(VULKAN_SDK)
STB_INCLUDE_PATH = /usr/local/include/

IDIR = include
ODIR = obj
SDIR = src

CC=g++
CFLAGS = -std=c++17 -I$(VULKAN_SDK_PATH)/include -I$(STB_INCLUDE_PATH) -I$(IDIR)
LDFLAGS = -L$(VULKAN_SDK_PATH)/lib `pkg-config --static --libs glfw3` -lvulkan

_DEPS = evulkan.h vertex.h cube.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = buffer.o descriptor.o instance.o main.o render.o run.o swap.o cube.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

Cubes: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

.PHONY: test clean

$(ODIR)/%.o: $(SDIR)/%.cpp $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

test: Cubes
	LD_LIBRARY_PATH=$(VULKAN_SDK_PATH)/lib VK_LAYER_PATH=$(VULKAN_SDK_PATH)/etc/vulkan/explicit_layer.d ./Cubes

clean:
	rm -f Cubes
	rm -f $(ODIR)/*.o
