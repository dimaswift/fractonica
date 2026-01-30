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
        typedef void (*ImGuiCallback)();
        SFMLDisplay(uint16_t width, uint16_t height, uint16_t scale, ImGuiInput *input);
        ~SFMLDisplay();
        bool begin() override;
        void print(const char *msg, int16_t x, int16_t y, uint8_t size) override;
        void log(const char *msg) override;
        void logError(const char *msg) override;
        void flush() override;
        void drawPixel(uint16_t x, uint16_t y, uint32_t color) override;
        void drawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color) override;
        void drawRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) override;
        void drawBitmap(int16_t x, int16_t y, uint16_t width, uint16_t height, const uint16_t *bitmap) override;

        static void addGuiCallback(ImGuiCallback callback);

        void update() override;
        bool isOpen() override;
        void clear() override;
        Vector2 size() override;
        uint32_t getColor(uint8_t r, uint8_t g, uint8_t b) const override;
        uint32_t getColorHSV(uint16_t h, uint8_t s, uint8_t v) const override;
    };

}
#endif // PC_DISPLAY_H
