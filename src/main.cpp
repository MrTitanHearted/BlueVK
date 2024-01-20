#include <iostream>

#include <engine.hpp>

int main() {
    bluevk::BlueVKEngine::Initialize(bluevk::BlueVKEngineParams{});

    bluevk::BlueVKEngine &engine = bluevk::BlueVKEngine::getInstance();

    engine.run();

    bluevk::BlueVKEngine::Shutdown();
    return EXIT_SUCCESS;
}