#pragma once
#include "Common.h"

// Validation layer variables
const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

class Instance
{
public:
	Instance(std::string applicationName,
		std::vector<const char*>& requiredInstanceExtensions);
	~Instance();

	Instance(const Instance&) = delete;
	Instance(Instance&&) = delete;

	// - Getters
	VkInstance handle() const;

private:

	VkInstance mHandle{VK_NULL_HANDLE};

	VkDebugUtilsMessengerEXT mDebugMessenger;

	// - Instance Creation
	bool checkInstanceExtensionSupport(std::vector<const char*>* checkExtensions);

	// - Validation Layer setup
	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
	bool checkValidationLayerSupport();
	void setupDebugMessenger();

	VkResult CreateDebugUtilsMessengerEXT(
		VkInstance instance,
		const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
		const VkAllocationCallbacks* pAllocator,
		VkDebugUtilsMessengerEXT* pDebugMessenger);

	void DestroyDebugUtilsMessengerEXT(
		VkInstance instance,
		VkDebugUtilsMessengerEXT debugMessenger,
		const VkAllocationCallbacks* pAllocator);
	 
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData);
};

