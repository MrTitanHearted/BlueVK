#include <iostream>

#include <BlueVK/BlueVK.hpp>

#include <SFML/Graphics.hpp>

const int WIDTH = 1700;
const int HEIGHT = 1000;
const std::string TITLE = "BlueVK";

int main() {
    sf::RenderWindow window = sf::RenderWindow{
        sf::VideoMode{WIDTH, HEIGHT},
        TITLE,
        sf::Style::Close | sf::Style::Titlebar,
        sf::ContextSettings(0)};

    BlueVK::Init({
        .windowTitle = TITLE,
        .windowSize = VkExtent2D{WIDTH, HEIGHT},
        .isResizable = false,
        .windowHandle = (void *)window.getSystemHandle(),
    });

    sf::Event event;
    sf::Clock clock;

    while (window.isOpen()) {
        float dt = clock.getElapsedTime().asMilliseconds();
        while (window.pollEvent(event)) {
            switch (event.type) {
                case sf::Event::Closed: {
                    window.close();
                } break;
                case sf::Event::KeyPressed:
                    switch (event.key.code) {
                        case sf::Keyboard::Escape: {
                            window.close();
                        } break;
                        default:
                            break;
                    }
                    break;
                default:
                    break;
            }
        }
    }

    BlueVK::Shutdown();
    return 0;
}