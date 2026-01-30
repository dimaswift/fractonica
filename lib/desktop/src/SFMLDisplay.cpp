#include "SFMLDisplay.h"
#include <SFML/Graphics.hpp>
#include <imgui-SFML.h>
#include "imgui.h"
#include <ImGuiInput.h>
#include <LunarClockApp.h>

#include "Utils.h"

namespace Fractonica
{
    SFMLDisplay::SFMLDisplay(uint16_t width, uint16_t height, uint16_t scale, ImGuiInput *input) : width(width),
                                                                                                   height(height),
                                                                                                   scale(scale),
                                                                                                   window(sf::VideoMode(sf::Vector2u(width * scale, height * scale), 2), "Fractonica"),
                                                                                                   sprite(texture),
                                                                                                   input(input)
    {
    }

    SFMLDisplay::~SFMLDisplay() {
        ImGui::SFML::Shutdown();
    }

    static sf::Color rgb565to888(uint16_t color)
    {
        uint8_t r = (color & 0xF800) >> 8;
        uint8_t g = (color & 0x07E0) >> 3;
        uint8_t b = (color & 0x001F) << 3;
        return {r, g, b};
    }


    bool SFMLDisplay::begin()
    {
        buffer = sf::Image(sf::Vector2u(width, height), sf::Color::Black);
        blackBuffer = sf::Image(sf::Vector2u(width, height), sf::Color::Black);
        if (texture.loadFromImage(buffer))
        {
            sprite = sf::Sprite(texture);
            sprite.setScale(sf::Vector2f(scale, scale));
        }
        return ImGui::SFML::Init(window);
    
    }
    void SFMLDisplay::print(const char *msg, int16_t x, int16_t y, uint8_t size)
    {
    }
    void SFMLDisplay::log(const char *msg)
    {
    }
    void SFMLDisplay::logError(const char *msg)
    {
    }
    void SFMLDisplay::flush()
    {
    }

    void SFMLDisplay::drawPixel(uint16_t x, uint16_t y, uint32_t color) {
        if(x >= width || y >= height) return;
        buffer.setPixel(sf::Vector2u(x, y), rgb565to888(color));
    }

    void SFMLDisplay::drawBitmap(int16_t x, int16_t y, uint16_t width, uint16_t height, const uint16_t *bitmap)
    {
    }

    void SFMLDisplay::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {
        int16_t dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
        int16_t dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
        int16_t err = dx + dy, e2;

        while (true)
        {
            drawPixel(x0, y0, color);
            if (x0 == x1 && y0 == y1)
                break;
            e2 = 2 * err;
            if (e2 >= dy)
            {
                err += dy;
                x0 += sx;
            }
            if (e2 <= dx)
            {
                err += dx;
                y0 += sy;
            }
        }
    }

    void SFMLDisplay::drawRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {

    }

    static SFMLDisplay::ImGuiCallback guiCallback = nullptr;

    void SFMLDisplay::addGuiCallback(ImGuiCallback callback) {
        guiCallback = callback;
    }

    void SFMLDisplay::update()
    {
        while (const std::optional event = window.pollEvent())
        {
            ImGui::SFML::ProcessEvent(window, *event);

            if (event->is<sf::Event::Closed>())
            {
                window.close();
                return;
            }

            if (const auto *keyPressed = event->getIf<sf::Event::KeyPressed>())
            {
                if (keyPressed->scancode == sf::Keyboard::Scancode::Escape) {
                    window.close();
                    return;
                }
            }
        }

        ImGui::SFML::Update(window, deltaClock.restart());

        if (guiCallback) {
            guiCallback();
        }

        window.clear();

        if (texture.loadFromImage(buffer))
        {
            window.draw(sprite);
        }

        ImGui::SFML::Render(window);

        window.display();
    }

    bool SFMLDisplay::isOpen()
    {
        return window.isOpen();
    }

    void SFMLDisplay::clear()
    {
        if (!buffer.copy(blackBuffer, sf::Vector2u(0, 0)))
        {
            return;
        }
    }

    Vector2 SFMLDisplay::size() {
        return Vector2{.x = (int16_t)width, .y = (int16_t) height};
    }

    uint32_t SFMLDisplay::getColor(uint8_t r, uint8_t g, uint8_t b) const {
        return Utils::Color(r, g, b);
    }

    uint32_t SFMLDisplay::getColorHSV(uint16_t h, uint8_t s, uint8_t v) const {
        return Utils::ColorHSV(h, s, v);
    }
};
