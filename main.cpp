#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <optional>
#include <stdexcept>
#include <vector>
#include <cstring>
#include <cstdlib>

using namespace std;

const uint32_t WIDTH = 1366;
const uint32_t HEIGHT = 768;

const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

class HelloTriangleApplication {
public:
    void run() {
        initWindow();
        initVulkan();
        mainLoop();
        cleanup();
    }

private:
    GLFWwindow* window;

    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

    struct QueueFamilyIndices{
        std::optional<uint32_t> graphicsFamily;
        bool isComplete(){
            return graphicsFamily.has_value();
        }
    };

    void initWindow() {
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
    }

    void initVulkan() {
        createInstance();
        setupDebugMessenger();
        pickPhysicalDevice();


    }

    void mainLoop() {
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
        }
    }

    void cleanup() {
        if (enableValidationLayers) {
            DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
        }

        vkDestroyInstance(instance, nullptr);

        glfwDestroyWindow(window);

        glfwTerminate();
    }

    void createInstance() {
        if (enableValidationLayers && !checkValidationLayerSupport()) {
            throw std::runtime_error("validation layers requested, but not available!");
        }

        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Hello Triangle";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        printAvailableExtensions(nullptr);
        auto extensions = getRequiredExtensions();
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();

        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
        if (enableValidationLayers) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();

            populateDebugMessengerCreateInfo(debugCreateInfo);
            createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
        } else {
            createInfo.enabledLayerCount = 0;
            
            createInfo.pNext = nullptr;
        }

        if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
            throw std::runtime_error("failed to create instance!");
        }
    }

    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
        createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = debugCallback;
    }

    void setupDebugMessenger() {
        if (!enableValidationLayers) return;

        VkDebugUtilsMessengerCreateInfoEXT createInfo;
        populateDebugMessengerCreateInfo(createInfo);

        if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
            throw std::runtime_error("failed to set up debug messenger!");
        }
    }

    std::vector<const char*> getRequiredExtensions() {
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

        if (enableValidationLayers) {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        return extensions;
    }

    bool checkValidationLayerSupport() {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        for (const char* layerName : validationLayers) {
            bool layerFound = false;

            for (const auto& layerProperties : availableLayers) {
                if (strcmp(layerName, layerProperties.layerName) == 0) {
                    layerFound = true;
                    break;
                }
            }

            if (!layerFound) {
                return false;
            }
        }

        return true;
    }

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

        return VK_FALSE;
    }

    void pickPhysicalDevice() {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
        if (deviceCount == 0) {
            throw std::runtime_error("failed to find GPUs with Vulkan support!");
        }
        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

        for (const auto& device : devices) {
            printPhysicalDeviceProperties(device);
            printPhysicalDeviceFeatures(device);
            printPhysicalDeviceQueueFamilies(device);
            
            if (isDeviceSuitable(device)) {
                VkPhysicalDeviceProperties deviceProperties;
                vkGetPhysicalDeviceProperties(device, &deviceProperties);
                cout << "Using GPU: " << deviceProperties.deviceName << endl;
                physicalDevice = device;
                break;
            }
        }

        if (physicalDevice == VK_NULL_HANDLE) {
            throw std::runtime_error("failed to find a suitable GPU!");
        }
    }
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device){
        QueueFamilyIndices indices;
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
        int i = 0;
        for (const auto& queueFamily : queueFamilies) {
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                indices.graphicsFamily = i;
                break;
            }

            i++;
        }
        return indices;
    }

    bool isDeviceSuitable(VkPhysicalDevice device) {
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(device, &deviceProperties);
        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
        return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
           deviceFeatures.geometryShader;
    }

    void printAvailableExtensions(const char* layer){
        uint32_t extensions_count;
        uint32_t result = vkEnumerateInstanceExtensionProperties( nullptr, &extensions_count, nullptr);
        std::vector<VkExtensionProperties> available_extensions( extensions_count );
        result = vkEnumerateInstanceExtensionProperties( layer, &extensions_count, &available_extensions[0] );
        if( (result != VK_SUCCESS) || (extensions_count == 0) ) {
          throw std::runtime_error("Could not enumerate Instance extensions.");
        }

        
        cout << "Available extensions from layer " << (layer == nullptr ? "default" : layer) << ":" << endl;
        for (auto ae : available_extensions){
            cout << "\t" << ae.extensionName << endl;
        }
        cout << endl;
    }
    void printPhysicalDeviceProperties(const VkPhysicalDevice &dev){
        VkPhysicalDeviceProperties device;
        vkGetPhysicalDeviceProperties(dev, &device);
        cout << "Device id:     " << device.deviceID << endl;
        cout << "Device name:   " << device.deviceName << endl;
        cout << "Device type:   " ;
        int dt = device.deviceType;
        if (dt == VK_PHYSICAL_DEVICE_TYPE_OTHER){
            cout << "unknown type" << endl;
        } else if (dt == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU){
            cout << "Integrated GPU" << endl;
        } else if (dt == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU){
            cout << "Discrete GPU" << endl;
        } else if (dt == VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU){
            cout << "Virtual GPU" << endl;
        } else if (dt == VK_PHYSICAL_DEVICE_TYPE_CPU){
            cout << "CPU" << endl;
        }
        cout << "API Verson:    " << device.apiVersion << endl << endl;
    }

    void printPhysicalDeviceQueueFamilies(const VkPhysicalDevice &dev){
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(dev, &queueFamilyCount, nullptr);

        vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(dev, &queueFamilyCount, queueFamilies.data());

        for (const auto& queueFamily : queueFamilies) {
            cout << "Found family of types: "<< endl;
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT){
                cout << "\tGRAPHICS" << endl;
            }
            if (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT){
                cout << "\tCOMPUTE" << endl;
            }
            if (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT){
                cout << "\tTRANSFER" << endl;
            }
            if (queueFamily.queueFlags & VK_QUEUE_SPARSE_BINDING_BIT){
                cout << "\tSPARCE BINDING" << endl;
            }
            if (queueFamily.queueFlags & VK_QUEUE_PROTECTED_BIT){
                cout << "\tPROTECTED" << endl;
            }
            cout << "Max queue of this type: " << queueFamily.queueCount << endl;
            VkExtent3D v = queueFamily.minImageTransferGranularity;
            cout << "Min granularity supported for image transfer operations: " << v.width <<"x"<< v.height<<"x" << v.depth << endl << endl;

        }
        
    }

    void printPhysicalDeviceFeatures(const VkPhysicalDevice &dev){
        VkPhysicalDeviceFeatures device;
        vkGetPhysicalDeviceFeatures(dev, &device);
        cout << "robustBufferAccess;                     " << device.robustBufferAccess;                     
        cout << endl;
        cout << "fullDrawIndexUint32;                    " << device.fullDrawIndexUint32;                    
        cout << endl;
        cout << "imageCubeArray;                         " << device.imageCubeArray;                         
        cout << endl;
        cout << "independentBlend;                       " << device.independentBlend;                       
        cout << endl;
        cout << "geometryShader;                         " << device.geometryShader;                         
        cout << endl;
        cout << "tessellationShader;                     " << device.tessellationShader;                     
        cout << endl;
        cout << "sampleRateShading;                      " << device.sampleRateShading;                      
        cout << endl;
        cout << "dualSrcBlend;                           " << device.dualSrcBlend;                           
        cout << endl;
        cout << "logicOp;                                " << device.logicOp;                                
        cout << endl;
        cout << "multiDrawIndirect;                      " << device.multiDrawIndirect;                      
        cout << endl;
        cout << "drawIndirectFirstInstance;              " << device.drawIndirectFirstInstance;              
        cout << endl;
        cout << "depthClamp;                             " << device.depthClamp;                             
        cout << endl;
        cout << "depthBiasClamp;                         " << device.depthBiasClamp;                         
        cout << endl;
        cout << "fillModeNonSolid;                       " << device.fillModeNonSolid;                       
        cout << endl;
        cout << "depthBounds;                            " << device.depthBounds;                            
        cout << endl;
        cout << "wideLines;                              " << device.wideLines;                              
        cout << endl;
        cout << "largePoints;                            " << device.largePoints;                            
        cout << endl;
        cout << "alphaToOne;                             " << device.alphaToOne;                             
        cout << endl;
        cout << "multiViewport;                          " << device.multiViewport;                          
        cout << endl;
        cout << "samplerAnisotropy;                      " << device.samplerAnisotropy;                      
        cout << endl;
        cout << "textureCompressionETC2;                 " << device.textureCompressionETC2;                 
        cout << endl;
        cout << "textureCompressionASTC_LDR;             " << device.textureCompressionASTC_LDR;             
        cout << endl;
        cout << "textureCompressionBC;                   " << device.textureCompressionBC;                   
        cout << endl;
        cout << "occlusionQueryPrecise;                  " << device.occlusionQueryPrecise;                  
        cout << endl;
        cout << "pipelineStatisticsQuery;                " << device.pipelineStatisticsQuery;                
        cout << endl;
        cout << "vertexPipelineStoresAndAtomics;         " << device.vertexPipelineStoresAndAtomics;         
        cout << endl;
        cout << "fragmentStoresAndAtomics;               " << device.fragmentStoresAndAtomics;               
        cout << endl;
        cout << "shaderTessellationAndGeometryPointSize; " << device.shaderTessellationAndGeometryPointSize; 
        cout << endl;
        cout << "shaderImageGatherExtended;              " << device.shaderImageGatherExtended;              
        cout << endl;
        cout << "shaderStorageImageExtendedFormats;      " << device.shaderStorageImageExtendedFormats;      
        cout << endl;
        cout << "shaderStorageImageMultisample;          " << device.shaderStorageImageMultisample;          
        cout << endl;
        cout << "shaderStorageImageReadWithoutFormat;    " << device.shaderStorageImageReadWithoutFormat;    
        cout << endl;
        cout << "shaderStorageImageWriteWithoutFormat;   " << device.shaderStorageImageWriteWithoutFormat;   
        cout << endl;
        cout << "shaderUniformBufferArrayDynamicIndexing;" << device.shaderUniformBufferArrayDynamicIndexing;
        cout << endl;
        cout << "shaderSampledImageArrayDynamicIndexing; " << device.shaderSampledImageArrayDynamicIndexing; 
        cout << endl;
        cout << "shaderStorageBufferArrayDynamicIndexing;" << device.shaderStorageBufferArrayDynamicIndexing;
        cout << endl;
        cout << "shaderStorageImageArrayDynamicIndexing; " << device.shaderStorageImageArrayDynamicIndexing; 
        cout << endl;
        cout << "shaderClipDistance;                     " << device.shaderClipDistance;                     
        cout << endl;
        cout << "shaderCullDistance;                     " << device.shaderCullDistance;                     
        cout << endl;
        cout << "shaderFloat64;                          " << device.shaderFloat64;                          
        cout << endl;
        cout << "shaderInt64;                            " << device.shaderInt64;                            
        cout << endl;
        cout << "shaderInt16;                            " << device.shaderInt16;                            
        cout << endl;
        cout << "shaderResourceResidency;                " << device.shaderResourceResidency;                
        cout << endl;
        cout << "shaderResourceMinLod;                   " << device.shaderResourceMinLod;                   
        cout << endl;
        cout << "sparseBinding;                          " << device.sparseBinding;                          
        cout << endl;
        cout << "sparseResidencyBuffer;                  " << device.sparseResidencyBuffer;                  
        cout << endl;
        cout << "sparseResidencyImage2D;                 " << device.sparseResidencyImage2D;                 
        cout << endl;
        cout << "sparseResidencyImage3D;                 " << device.sparseResidencyImage3D;                 
        cout << endl;
        cout << "sparseResidency2Samples;                " << device.sparseResidency2Samples;                
        cout << endl;
        cout << "sparseResidency4Samples;                " << device.sparseResidency4Samples;                
        cout << endl;
        cout << "sparseResidency8Samples;                " << device.sparseResidency8Samples;                
        cout << endl;
        cout << "sparseResidency16Samples                " << device.sparseResidency16Samples;
        cout << endl;
        cout << "sparseResidencyAliased;                 " << device.sparseResidencyAliased;                 
        cout << endl;
        cout << "variableMultisampleRate;                " << device.variableMultisampleRate;                
        cout << endl;
        cout << "inheritedQueries;                       " << device.inheritedQueries;                       
        cout << endl;
        cout << endl;
    }
};

int main() {
    HelloTriangleApplication app;

    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
