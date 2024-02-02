#include <BlueVK/Platform/Platform.hpp>

#ifdef BLUE_PLATFORM_LINUX
#include <X11/Xlib.h>
#include <vulkan/vulkan_xlib.h>

VkSurfaceKHR BlueVKCreateSurface(VkInstance instance, void *windowHandle) {
    VkXlibSurfaceCreateInfoKHR info{
        .sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR,
        .dpy = XOpenDisplay(nullptr),
        .window = (Window)windowHandle,
    };

    VkSurfaceKHR surface;

    VK_CHECK(vkCreateXlibSurfaceKHR(instance, &info, nullptr, &surface));

    return surface;
}

#endif