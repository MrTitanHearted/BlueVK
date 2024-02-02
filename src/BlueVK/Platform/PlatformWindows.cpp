#include <BlueVK/Platform/Platform.hpp>

#ifdef BLUE_PLATFORM_WINDOWS
#include <windows.h>
#include "vulkan_win32.h"

VkSurfaceKHR BlueVKCreateSurface(VkInstance instance, void *windowHandle) {
    VkWin32SurfaceCreateInfoKHR info{
        .sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR,
        .hinstance = GetModuleHandle(nullptr),
        .hwnd = (HWND)windowHandle,
    };

    VkSurfaceKHR surface;

    VK_CHECK(vkCreateWin32SurfaceKHR(instance, &info, nullptr, &surface));

    return surface;
}

#endif