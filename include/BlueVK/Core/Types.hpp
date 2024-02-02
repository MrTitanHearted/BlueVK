#pragma once

#include <vector>
#include <string>
#include <memory>
#include <functional>
#include <optional>

#include <vulkan/vulkan.h>
#include <vulkan/vk_enum_string_helper.h>
#include <vk_mem_alloc.h>

#include <fmt/core.h>

#include <glm/glm.hpp>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
#define BLUE_PLATFORM_WINDOWS 1
#ifndef _WIN64
#error "64-bit is required on Windows!"
#endif
#elif defined(__linux__) || defined(__gnu_linux__)
// Linux OS
#define BLUE_PLATFORM_LINUX 1
#if defined(__ANDROID__)
#define BLUE_PLATFORM_ANDROID 1
#endif
#elif defined(__unix__)
// Catch anything not caught by the above.
#define BLUE_PLATFORM_UNIX 1
#elif defined(_POSIX_VERSION)
// Posix
#define BLUE_PLATFORM_POSIX 1
#elif __APPLE__
// Apple Platforms
#define BLUE_PLATFORM_APPLE 1
#include <TargetConditionals.h>
#if TARGET_IPHONE_SIMULATOR
// iOS Simulator
#define BLUE_PLATFORM_IOS 1
#define BLUE_PLATFORM_IOS_SIMULATOR 1
#elif TARGET_OS_IPHONE
#define BLUE_PLATFORM_IOS 1
// iOS device
#elif TARGET_OS_MAC
// Other kinds of Mac OS
#else
#error "Unknown Apple Platform"
#endif
#else
#error "Unknown Platform!"
#endif

#define VK_CHECK(x)                                                                                   \
    do {                                                                                              \
        VkResult err = x;                                                                             \
        if (err) {                                                                                    \
            throw std::runtime_error(fmt::format("[BlueVK]::[ERROR]: Detected Vulkan error: {}", string_VkResult(err))); \
        }                                                                                             \
    } while (0)
