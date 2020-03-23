VULKAN_SDK_PATH = $(VULKAN_SDK)
STB_INCLUDE_PATH = /usr/local/include/

CFLAGS = -std=c++17 -I$(VULKAN_SDK_PATH)/include -I$(STB_INCLUDE_PATH)
LDFLAGS = -L$(VULKAN_SDK_PATH)/lib `pkg-config --static --libs glfw3` -lvulkan

HelloTriangle: main.cpp
	g++ $(CFLAGS) -o HelloTriangle descriptor.cpp rest.cpp main.cpp $(LDFLAGS)

.PHONY: test clean

test: HelloTriangle
	LD_LIBRARY_PATH=$(VULKAN_SDK_PATH)/lib VK_LAYER_PATH=$(VULKAN_SDK_PATH)/etc/vulkan/explicit_layer.d ./HelloTriangle

clean:
	rm -f HelloTriangle
