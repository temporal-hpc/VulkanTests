VULKAN_SDK_PATH = /opt/vulkan/1.2.135.0/x86_64

CFLAGS = -std=c++17 -I$(VULKAN_SDK_PATH)/include
LDFLAGS = -lvulkan -lglfw -L$(VULKAN_SDK_PATH)/lib
#LDFLAGS =  `pkg-config --static --libs glfw3` -lvulkan


VulkanTest: main.cpp
	g++ $(CFLAGS) -o VulkanTest main.cpp $(LDFLAGS)

.PHONY: test clean

test: VulkanTest
	./VulkanTest

clean:
	rm -f VulkanTest
