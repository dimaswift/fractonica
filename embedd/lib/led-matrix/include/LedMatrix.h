#include <Arduino.h>
#include <OctalGlyph.h>
#include <Utils.h>
#include "WS2812Matrix.h"
#include "saros.h"

Fractonica::WS2812Matrix matrix(4, 255, 16, 16, Fractonica::IMatrix::BottomLeft);
Fractonica::OctalGlyphSettings glyph_settings;

int saros = 141;

uint64_t period = 0;
uint64_t counter =0;


void setup()
{

    Serial.begin(9600);

    matrix.begin();

    matrix.clear();

    glyph_settings.showBorder = false;

    period = get_solar_saros_period_duration_ms(1774616903, saros, 10);


}

void drawQuad(uint32_t color) {
    Fractonica::OctalGlyph::Draw(counter, &matrix, Vector2(9,9), 7, color);

    Fractonica::OctalGlyph::Draw(counter / 07777, &matrix, Vector2(9,0), 7, color);

    Fractonica::OctalGlyph::Draw(counter / 077777777, &matrix, Vector2(0,0), 7, color);

    Fractonica::OctalGlyph::Draw(counter / 0777777777777, &matrix, Vector2(0,9), 7, color);
}

void drawSingle(uint32_t color) {
    Fractonica::OctalGlyph::Draw(counter, &matrix, Vector2(0,0), 15, color);
}

void loop()
{
    uint32_t color = matrix.getColorHSV(counter,255,255);

    counter++;

    matrix.clear();

    drawSingle(color);

    matrix.flush();

    delay(period);
}
