VULKAN_SDK_PATH = $(VULKAN_SDK)
STB_INCLUDE_PATH = /usr/local/include/

IDIR = include
ODIR = obj
SDIR = src

CC=g++
CFLAGS = -std=c++17 -I$(VULKAN_SDK_PATH)/include -I$(STB_INCLUDE_PATH) -I$(IDIR)
LDFLAGS = -L$(VULKAN_SDK_PATH)/lib `pkg-config --static --libs glfw3` -lvulkan
OPTFLAGS = -O3

_DEPS = evulkan.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = buffer.o descriptor.o instance.o main.o render.o run.o swap.o model.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

HelloTriangle: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(OPTFLAGS)

.PHONY: test clean

$(ODIR)/%.o: $(SDIR)/%.cpp $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

test: HelloTriangle
	LD_LIBRARY_PATH=$(VULKAN_SDK_PATH)/lib VK_LAYER_PATH=$(VULKAN_SDK_PATH)/etc/vulkan/explicit_layer.d ./HelloTriangle

clean:
	rm -f HelloTriangle
	rm -f $(ODIR)/*.o
