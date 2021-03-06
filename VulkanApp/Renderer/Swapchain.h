#pragma once
#include "Common.h"

class Device;

// Supported surface capabilities,formats and presentation modes
struct SurfaceSupport {
	VkSurfaceCapabilitiesKHR surfaceCapabilities;		// Surface properties e.g. image size/extent
	std::vector<VkSurfaceFormatKHR> formats;			// Surface image formats e.g. RGBA and size of each
	std::vector<VkPresentModeKHR> presentationModes;	// How images should be presented to screen
};

// Properties of a swapchain
struct SwapchainDetails {
	uint32_t imageCount;
	VkExtent2D extent;
	VkSurfaceFormatKHR surfaceFormat;
	VkPresentModeKHR presentMode;
	VkImageUsageFlags usage;
};

const uint32_t IMAGE_COUNT_MAILBOX = 3;
const uint32_t IMAGE_COUNT_DEFAULT = 2;

class Swapchain
{
public:

	// TODO: need to be able to recreate swapchain and change priorities before creation
	Swapchain(Device& device, 
		const VkExtent2D& newExtent, 
		VkSurfaceKHR surface,
		VkPresentModeKHR presentMode,
		VkImageUsageFlags usage);
	~Swapchain();

	// - Getters
	Device& device() const;
	VkSwapchainKHR handle() const;
	const SwapchainDetails& details() const;
	const VkExtent2D& extent() const;
	VkFormat format() const;
	VkImageUsageFlags usage() const;
	const std::vector<VkImage>& images() const;
	uint32_t imageCount() const;

	// - Setters
	void setPresentationPriority(const std::vector<VkPresentModeKHR>& newList);
	void setSurfaceFormatPriority(const std::vector<VkSurfaceFormatKHR>& newList);

	// - Image Management
	VkResult acquireNextImageIndex(VkSemaphore imageAvailable, uint32_t& imageIndex);

	
private:

	Device& mDevice;


	VkSurfaceKHR mSurface;
	VkSwapchainKHR mHandle{VK_NULL_HANDLE};
	SwapchainDetails mDetails;
	
	std::vector<VkImage> mImages;

	
	// - Presentation mode priority  (vector[0] has high priority, vector[size-1] has low priority)
	std::vector<VkPresentModeKHR> mPresentationModePriority = {
		VK_PRESENT_MODE_MAILBOX_KHR,				// 0. triple buffer
		VK_PRESENT_MODE_FIFO_RELAXED_KHR,			// 1. relaxed double buffer
		VK_PRESENT_MODE_FIFO_KHR,					// 2. double buffer
		VK_PRESENT_MODE_IMMEDIATE_KHR};				// 3. Draw when ready

	// - Surface format priority (vector[0] has high priority, vector[size-1] has low priority)
	std::vector<VkSurfaceFormatKHR> mSurfaceFormatPriorityList = {
		{VK_FORMAT_R8G8B8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR},
		{VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR},
		{VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR},
		{VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR} };

	// - Swapchain creation
	//void createSwapchain(const VkExtent2D& newExtent);
	// -- Support
	void getSurfaceSupport(SurfaceSupport& swapchainSupport);
	void chooseSurfaceFormat(std::vector<VkSurfaceFormatKHR>& formats);
	void choosePresentationMode(std::vector<VkPresentModeKHR>& supportedPresentationModes, VkPresentModeKHR requestedPresentationMode);
	void chooseImageCount();
	void chooseExtent(VkSurfaceCapabilitiesKHR& supportedSurfaceCapabilities, const VkExtent2D& requestedExtent);


};

