#include <Arduino.h>
#include <OctalGlyph.h>
#include "SSD1306Display.h"
#include "saros.h"
#include "RotaryEncoder.h"
#include "WifiClock.h"
#include "EventLogger.h"
#include "I2SRecorder.h"
#include "SDMMCFileSystem.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define SCL_PIN 47
#define SDA_PIN 21
#define SCREEN_ADDRESS 0x3C

#define ENCODER_PIN_IN1 1
#define ENCODER_PIN_IN2 2

#define PIN_BTN_CONFIRM 41
#define PIN_BTN 42     // button (to GND when pressed)

#define I2S_BCLK_PIN 5 // Bit Clock
#define I2S_LRC_PIN 6  // Word Select / Left-Right Clock
#define I2S_DOUT_PIN 4 // Data Out
#define SAMPLE_RATE 44100

Fractonica::SSD1306Display display(SCREEN_WIDTH, SCREEN_HEIGHT, SCL_PIN, SDA_PIN, SCREEN_ADDRESS);
Fractonica::OctalGlyphSettings glyph_settings;
Fractonica::WifiClock wifiClock("RT-GPON-7", "857010486557");
Fractonica::SDMMCFileSystem sdcard;
Fractonica::I2SRecorder recorder(&sdcard, 13, 12, 14);
Fractonica::EventLogger logger(141, &wifiClock, &sdcard, &recorder);
RotaryEncoder encoder(ENCODER_PIN_IN1, ENCODER_PIN_IN2, RotaryEncoder::LatchMode::FOUR0);

volatile int saros = 141;
volatile uint64_t sarosNumDisplayTime = 0;
volatile uint64_t prevBin = 0;

#define DEBOUNCE_BTN(pin, debounce_ms) \
    ([]() -> bool {                                                 \
static uint32_t lastChangeMs = 0;                          \
static bool     lastStable   = false;                      \
static bool     lastRaw      = false;                      \
\
bool rawPressed = (digitalRead(pin) == LOW);               \
\
if (rawPressed != lastRaw) {                               \
lastRaw      = rawPressed;                             \
lastChangeMs = millis();                               \
}                                                           \
\
if ((millis() - lastChangeMs) > (debounce_ms)) {          \
if (lastStable != rawPressed) {                        \
lastStable = rawPressed;                           \
if (lastStable) return true;                       \
}                                                       \
}                                                           \
return false; }())

void logStatus(const char *msg, int delay_ = 0)
{
    display.clear();
    display.print(msg, 0, 0, 1);
    display.flush();
    if (delay_ > 0)
        delay(delay_);
}

void encoderTask(void * pvParameters) {
    static int pos = 0;

    for (;;) {
        encoder.tick();
        int newPos = encoder.getPosition();
        if (pos != newPos) {
            int dir = (int)encoder.getDirection();
            int newSaros = saros - dir;
            if (newSaros > OLDEST_SAROS) newSaros = OLDEST_SAROS;
            if (newSaros < YOUNGEST_SAROS) newSaros = YOUNGEST_SAROS;

            saros = newSaros;
            pos = newPos;
            prevBin = 0;
            sarosNumDisplayTime = millis() + 1000;
        }

        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

void setup()
{

    xTaskCreatePinnedToCore(
        encoderTask,
        "EncoderTask",
        4096,
        NULL,
        1,
        NULL,
        0
    );

    Serial.begin(9600);

    delay(500);

    wifiClock.begin();

    if (!display.begin())
    {
        Serial.println(F("SSD1306 allocation failed"));
        for (;;)
            ;
    }

    Serial.println(F("SSD1306 allocated"));

    pinMode(PIN_BTN, INPUT_PULLUP);
    pinMode(PIN_BTN_CONFIRM, INPUT_PULLUP);

    glyph_settings.showBorder = false;

    if (sdcard.begin())
    {
        if (!recorder.begin())
        {
            logStatus("Recorder failed", 1000);
        }
    }
    else
    {
        logStatus("SD card failed", 1000);
        for (;;)
            ;
    }
}

void loop()
{
    wifiClock.update();

    uint64_t bin = calculate_solar_octal_phase(wifiClock.now(), saros, 2);

    if (prevBin != bin) {
        display.clear();
        Fractonica::OctalGlyph::Draw(bin, &display, Vector2(0, 0), glyph_settings);
        prevBin = bin;
    }

    recorder.loop(2);

    if (logger.recording())
    {
        display.print("Recording", 0, 100, 1);
    }

    if (millis() < sarosNumDisplayTime) {
        char msg[16];
        sprintf(msg, "%d", saros);
        display.print(msg, 0, 100, 1);
    }

    if (DEBOUNCE_BTN(PIN_BTN, 25))
    {
        prevBin = 0;
        display.clear();
        if (logger.recording()) {
            if (!logger.endLog()) {
                logStatus("Failed to save", 1000);
            }

        }
        else {
            if (!logger.startLog()) {
                logStatus("Failed to record", 1000);
            }
        }
    }

    display.flush();
}
