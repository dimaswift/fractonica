#include <Arduino.h>
#include <OctalGlyph.h>
#include "SSD1306Display.h"
#include "saros.h"
#include "RotaryEncoder.h"
#include "I2SAudio.h"
#include "WifiClock.h"
#include "ToneGenerator.h"
#include "Synth.h"
#include <math.h>

#include "OV2640Camera.h"
#include "SDMMCStorage.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define SCL_PIN 47
#define SDA_PIN 21
#define SCREEN_ADDRESS 0x3C

#define ENCODER_PIN_IN1 1
#define ENCODER_PIN_IN2 2

#define  PIN_BTN 42 // button (to GND when pressed)
#define  I2S_BCLK_PIN 5 // Bit Clock
#define  I2S_LRC_PIN 6 // Word Select / Left-Right Clock
#define  I2S_DOUT_PIN 4 // Data Out
#define  SAMPLE_RATE 44100

Fractonica::ToneGenerator tone_generator(32, SAMPLE_RATE);
Fractonica::SSD1306Display display(SCREEN_WIDTH, SCREEN_HEIGHT, SCL_PIN, SDA_PIN, SCREEN_ADDRESS);
Fractonica::OctalGlyphSettings glyph_settings;
Fractonica::WifiClock wifiClock("RT-GPON-7", "857010486557");
Fractonica::I2SAudio audio;
Fractonica::Synth synth;
Fractonica::OV2640Camera camera;
Fractonica::FrameBuffer frameBuffer;
Fractonica::SDMMCStorage storage;

RotaryEncoder encoder(ENCODER_PIN_IN1, ENCODER_PIN_IN2, RotaryEncoder::LatchMode::TWO03);

float maxVolume = 1.0f;
uint64_t prevBin;
int channel;


int16_t generateSample() {
    return synth.Sample() * maxVolume;
}

IRAM_ATTR void checkPosition() {
    encoder.tick();
}

bool readButtonPressedDebounced() {
    static uint32_t lastChangeMs = 0;
    static bool lastStable = false;
    static bool lastRaw = false;

    bool rawPressed = (digitalRead(PIN_BTN) == LOW); // using INPUT_PULLUP

    if (rawPressed != lastRaw) {
        lastRaw = rawPressed;
        lastChangeMs = millis();
    }

    if (millis() - lastChangeMs > 25) {

        if (lastStable != rawPressed) {
            lastStable = rawPressed;
            if (lastStable)
                return true;
        }
    }
    return false;
}


void logStatus(const char* msg, int delay_ = 0) {
    display.clear();
    display.print(msg, 0, 0, 1);
    display.flush();
    if (delay_ > 0) delay(delay_);
}


void setup() {



    Serial.begin(9600);

    delay(500);

    tone_generator.Randomize(11, 66);

    if (audio.begin(I2S_BCLK_PIN, I2S_LRC_PIN, I2S_DOUT_PIN, SAMPLE_RATE, generateSample)) {
        Serial.println("Synth started on Core 0!");
    } else {
        Serial.println("Failed to start I2S.");
    }

    wifiClock.begin();

    if (!display.begin()) {
        Serial.println(F("SSD1306 allocation failed"));
        for (;;);
    }

    Serial.println(F("SSD1306 allocated"));

    pinMode(PIN_BTN, INPUT_PULLUP);
    glyph_settings.showBorder = false;


    if (!storage.begin()) {

        logStatus("Storage failed", 1000);
    }


}

void ModulateTone(uint32_t sample_counter, uint32_t base_phase_inc, int32_t base_vol, uint32_t duration_samples, int32_t & out_phase_inc, int32_t & out_vol) {
    tone_generator.ModulateFast(sample_counter, base_phase_inc, base_vol, duration_samples, out_phase_inc, out_vol);
}

void loop() {
    wifiClock.update();
    uint64_t bin = calculate_solar_octal_phase(wifiClock.now(), 141, 2);
    if (prevBin != bin) {
        int zeroes = __builtin_ctzll(bin);
        if (zeroes > 3) {
            channel++;
            if (channel >= synth.GetCapacity()) channel = 0;
            synth.PlayVoice(channel, 64.0f + (zeroes * 128), 1, pow(2, zeroes - 3), Fractonica::Synth::OscSine, ModulateTone);
        }
        display.clear();
        Fractonica::OctalGlyph::Draw(bin, &display, Vector2(0, 0), glyph_settings);
        display.flush();
        prevBin = bin;
    }

    static int pos = 0;

    encoder.tick();

    int newPos = encoder.getPosition();
    if (pos != newPos) {
        maxVolume -= 0.05f * (int) encoder.getDirection();
        if (maxVolume > 1.0f) maxVolume = 1.0f;
        if (maxVolume < 0.0f) maxVolume = 0.0f;
        display.clear();
        char msg[32];
        sprintf(msg, "%d", static_cast<int>(maxVolume * 100));
        display.print(msg, 0, 100, 2);
        display.flush();
        pos = newPos;
    }

    if (readButtonPressedDebounced()) {
        prevBin = 0;
        display.clear();
        synth.PlayVoice(0, 500, 1, 3, Fractonica::Synth::OscSine, ModulateTone);
        if (!camera.begin()) {
            logStatus("Camera failed to initialize", 3000);

        }
        else {
            logStatus("Camera ok", 1000);
        }

        while (!camera.isReady()) {
            delay(10);
        }

        if (camera.captureFrame(&frameBuffer)) {

            logStatus("Captured!", 1000);
            if (storage.isReady()) {
                bool saved = false;
                File f = storage.openForWrite("/frame.jpg");
                if (f) {
                    auto written = f.write(frameBuffer.buffer, frameBuffer.length);
                    f.close();
                    saved = written == frameBuffer.length;
                }

                if (saved) {
                    logStatus("Saved!", 1000);
                }
                else {
                    logStatus("Saving failed", 1000);
                }
            }
            else {
                logStatus("Storage not ready...", 1000);
            }

        }
        else {
            logStatus("Failed to capture", 3000);
        }


        camera.end();

    }
}
