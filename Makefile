VULKAN_SDK_PATH = $(VULKAN_SDK)
STB_INCLUDE_PATH = /usr/local/include/

IDIR = ./
ODIR = obj
SDIR = ./

CC=g++
CFLAGS = -std=c++17 -I$(VULKAN_SDK_PATH)/include -I$(STB_INCLUDE_PATH) -I$(IDIR)
LDFLAGS = -L$(VULKAN_SDK_PATH)/lib `pkg-config --static --libs glfw3` -lvulkan
OPTFLAGS = -O3

_DEPS = evulkan.cpp
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = main.o evulkan.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

Cube: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(OPTFLAGS)

.PHONY: test clean

$(ODIR)/%.o: $(SDIR)/%.cpp $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

test: Cube
	LD_LIBRARY_PATH=$(VULKAN_SDK_PATH)/lib VK_LAYER_PATH=$(VULKAN_SDK_PATH)/etc/vulkan/explicit_layer.d ./HelloTriangle

clean:
	rm -f Cube
	rm -f $(ODIR)/*.o
