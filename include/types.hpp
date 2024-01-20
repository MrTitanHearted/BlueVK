#pragma once

#include <vector>
#include <string>
#include <memory>
#include <functional>
#include <optional>
#include <deque>

#include <vulkan/vulkan.h>
#include <vulkan/vk_enum_string_helper.h>
#include <vk_mem_alloc.h>

#include <fmt/core.h>

#include <glm/glm.hpp>

namespace bluevk {
    struct BlueVKImage {
        VkImage image;
        VkImageView view;
        VmaAllocation allocation;
        VkExtent2D extent;
        VkFormat format;
    };
}

#define VK_CHECK(x)                                                        \
    do {                                                                   \
        VkResult err = x;                                                  \
        if (err) {                                                         \
            fmt::print("Detected Vulkan error: {}", string_VkResult(err)); \
            abort();                                                       \
        }                                                                  \
    } while (0)
