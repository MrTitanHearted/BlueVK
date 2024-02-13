#pragma once

#include <BlueVK/Core/Types.hpp>

struct DeletionQueue {
    std::vector<std::function<void()>> functions{};

    void push_back(std::function<void()> function) {
        functions.insert(functions.begin(), function);
    }

    void flush() {
        for (std::function<void()> func : functions) {
            func();
        }

        functions.clear();
    }
};