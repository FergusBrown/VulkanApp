#include "Surface.h"

#include "Instance.h"

// Constructor is passed a handle of an existing surface
// Surface creation is handle externally as different libraries may provides the surface
Surface::Surface(Instance& instance, VkSurfaceKHR handle) :
   mInstance(instance),  mHandle(handle)
{
    if (handle == VK_NULL_HANDLE)
    {
        throw std::runtime_error("Surface handle is NULL!");
    }
}

Surface::~Surface()
{
    if (mHandle != VK_NULL_HANDLE)
    {
        vkDestroySurfaceKHR(mInstance.handle(), mHandle, nullptr);
    }
}

VkSurfaceKHR Surface::handle() const
{
    return mHandle;
}
