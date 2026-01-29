#ifndef PC_DISPLAY_H
#define PC_DISPLAY_H
#include <IDisplay.h>
#include <SFML/Graphics.hpp>
#include <ImGuiInput.h>
namespace Fractonica
{

    class SFMLDisplay : public IDisplay
    {

    private:
        sf::RenderWindow window;
        sf::Image buffer;
        sf::Image blackBuffer;
        sf::Texture texture;
        sf::Sprite sprite;
        uint16_t width;
        uint16_t height;
        uint16_t scale;
        sf::Clock deltaClock;
        ImGuiInput *input;

    public:
        SFMLDisplay(uint16_t width, uint16_t height, uint16_t scale, ImGuiInput *input);
        ~SFMLDisplay();
        bool begin() override;
        void print(const char *msg, int16_t x, int16_t y, uint8_t size) override;
        void log(const char *msg) override;
        void logError(const char *msg) override;
        void flush() override;
        void drawPixel(int16_t x, int16_t y, uint16_t color) override;
        void drawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color) override;
        void drawBitmap(int16_t x, int16_t y, uint16_t width, uint16_t height, const uint16_t *bitmap) override;
        void update() override;
        bool isOpen() override;
        void clear() override;
        Vector2 size() override;
    };

}
#endif // PC_DISPLAY_H
