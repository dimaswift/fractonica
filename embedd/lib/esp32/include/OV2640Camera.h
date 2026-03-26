#ifndef PLATFORM_OV2640_CAMERA_H
#define PLATFORM_OV2640_CAMERA_H

#include "ICamera.h"
#include "esp_camera.h"
#include <Arduino.h>

namespace Fractonica {

    struct CameraPins {
        int pinPwdn = 1;      // Power down pin
        int pinReset = -1;     // Reset pin
        int pinXclk = 15;      // XCLK pin
        int pinSiod = 4;      // SIOD (I2C SDA)
        int pinSioc = 5;      // SIOC (I2C SCL)
        int pinD7 = 16;        // Data bit 7
        int pinD6 = 17;        // Data bit 6
        int pinD5  = 18;        // Data bit 5
        int pinD4 = 12;        // Data bit 4
        int pinD3 = 10;        // Data bit 3
        int pinD2 = 8;        // Data bit 2
        int pinD1 = 9;        // Data bit 1
        int pinD0 = 11;        // Data bit 0
        int pinVsync = 6;     // VSYNC
        int pinHref = 7;      // HREF
        int pinPclk = 13;      // PCLK
        int xclkFreqMhz = 20000000;  // XCLK frequency in MHz
    };
    /**
     * OV2640 Camera implementation for ESP32-S3
     * Adapts ESP32 camera library to platform-agnostic interface
     */
    class OV2640Camera : public ICamera {
    private:

        bool _initialized;
        uint32_t _frameCount;
        uint32_t _errorCount;
        camera_config_t _espConfig = {};
        camera_fb_t* _currentFrameBuffer;  // Keep track of the current frame buffer
        
        // Convert our frame size enum to ESP camera frame size
        static framesize_t toEspFrameSize(const FrameSize size) {
            switch (size) {
                case FrameSize::QQVGA: return FRAMESIZE_QQVGA;
                case FrameSize::QCIF: return FRAMESIZE_QCIF;
                case FrameSize::HQVGA: return FRAMESIZE_HQVGA;
                case FrameSize::QVGA: return FRAMESIZE_QVGA;
                case FrameSize::CIF: return FRAMESIZE_CIF;
                case FrameSize::VGA: return FRAMESIZE_VGA;
                case FrameSize::SVGA: return FRAMESIZE_SVGA;
                case FrameSize::XGA: return FRAMESIZE_XGA;
                case FrameSize::SXGA: return FRAMESIZE_SXGA;
                case FrameSize::UXGA: return FRAMESIZE_UXGA;
                default: return FRAMESIZE_SVGA;
            }
        }

        // Convert our pixel format enum to ESP camera pixel format
        static pixformat_t toEspPixelFormat(FrameFormat format);

        static FrameFormat fromEspPixelFormat(pixformat_t format) {
            switch (format) {
                case PIXFORMAT_JPEG: return FrameFormat::JPEG;
                case PIXFORMAT_RGB565: return FrameFormat::RGB565;
                case PIXFORMAT_RGB888: return FrameFormat::RGB888;
                case PIXFORMAT_GRAYSCALE: return FrameFormat::GRAYSCALE;
                case PIXFORMAT_YUV422: return FrameFormat::YUV422;
                default: return FrameFormat::JPEG;
            }
        }

    public:
        OV2640Camera(const CameraPins& pins = {}, bool hasPSRAM = true)
            : _initialized(false), _frameCount(0), _errorCount(0), _currentFrameBuffer(nullptr)
        {
            memset(&_espConfig, 0, sizeof(_espConfig));

            _espConfig.pin_pwdn = pins.pinPwdn;
            _espConfig.pin_reset = pins.pinReset;
            _espConfig.pin_xclk = pins.pinXclk;
            _espConfig.pin_sccb_sda = pins.pinSiod;
            _espConfig.pin_sccb_scl = pins.pinSioc;

            _espConfig.pin_d7 = pins.pinD7;
            _espConfig.pin_d6 = pins.pinD6;
            _espConfig.pin_d5 = pins.pinD5;
            _espConfig.pin_d4 = pins.pinD4;
            _espConfig.pin_d3 = pins.pinD3;
            _espConfig.pin_d2 = pins.pinD2;
            _espConfig.pin_d1 = pins.pinD1;
            _espConfig.pin_d0 = pins.pinD0;

            _espConfig.pin_vsync = pins.pinVsync;
            _espConfig.pin_href = pins.pinHref;
            _espConfig.pin_pclk = pins.pinPclk;

            _espConfig.xclk_freq_hz = pins.xclkFreqMhz;
            _espConfig.ledc_timer = LEDC_TIMER_0;
            _espConfig.ledc_channel = LEDC_CHANNEL_0;

            // Default configuration
            _espConfig.pixel_format = PIXFORMAT_JPEG;
            _espConfig.frame_size = FRAMESIZE_QVGA;
            _espConfig.jpeg_quality = 12;
            _espConfig.fb_count = 2;
            _espConfig.grab_mode = CAMERA_GRAB_LATEST;

            // Use PSRAM if available
            if (hasPSRAM)
            {
                _espConfig.fb_location = CAMERA_FB_IN_PSRAM;
            }
            else
            {
                _espConfig.fb_location = CAMERA_FB_IN_DRAM;
            }
        }

        ~OV2640Camera() override {
            OV2640Camera::end();
        }

        bool begin() override {
            return begin(CameraConfig{
                FrameFormat::GRAYSCALE,
                FrameSize::QQVGA,
                12,
                1
            });
        }

        bool begin(const CameraConfig& config) override {

            if (_initialized) {
                return true;
            }

        
            _espConfig.pixel_format = toEspPixelFormat(config.format);
            _espConfig.frame_size = toEspFrameSize(config.frameSize);
            _espConfig.jpeg_quality = config.jpegQuality;
            _espConfig.fb_count = config.frameBuffers;

            // Initialize camera
            esp_err_t err = esp_camera_init(&_espConfig);
            if (err != ESP_OK) {
                Serial.printf("Camera init failed with error 0x%x\n", err);
                _errorCount++;
                return false;
            }

            _initialized = true;
            _frameCount = 0;

            Serial.println("Camera initialized successfully");
            return true;
        }

        void end() override {
            if (_initialized) {
                // Release any pending frame buffer
                if (_currentFrameBuffer) {
                    esp_camera_fb_return(_currentFrameBuffer);
                    _currentFrameBuffer = nullptr;
                }
                esp_camera_deinit();
                _initialized = false;
            }
        }

        bool isReady() override {
            return _initialized;
        }

        CameraStatus getStatus() override {
            CameraStatus status{};
            status.initialized = _initialized;
            status.ready = _initialized;
            status.frameCount = _frameCount;
            status.errorCount = _errorCount;
            strncpy(status.model, "OV2640", sizeof(status.model) - 1);
            status.model[sizeof(status.model) - 1] = '\0';
            return status;
        }

        bool captureFrame(FrameBuffer* frame) override {
            if (!_initialized || !frame) {
                _errorCount++;
                return false;
            }

            // Release previous frame if not released
            if (_currentFrameBuffer) {
                esp_camera_fb_return(_currentFrameBuffer);
                _currentFrameBuffer = nullptr;
            }

            camera_fb_t* fb = esp_camera_fb_get();
            if (!fb) {
                Serial.println("Camera capture failed");
                _errorCount++;
                return false;
            }

            // Store the frame buffer pointer so we can return it later
            _currentFrameBuffer = fb;

            // Fill our platform-agnostic frame buffer
            frame->buffer = fb->buf;
            frame->length = fb->len;
            frame->width = fb->width;
            frame->height = fb->height;
            frame->format = fromEspPixelFormat(fb->format);
            frame->timestamp = millis();

            _frameCount++;

            return true;
        }

        bool releaseFrame(FrameBuffer* frame) override {
            if (!frame) {
                return false;
            }

            // Return the stored frame buffer to the camera driver
            if (_currentFrameBuffer) {
                esp_camera_fb_return(_currentFrameBuffer);
                _currentFrameBuffer = nullptr;
            }

            // Clear the frame buffer info
            frame->buffer = nullptr;
            frame->length = 0;

            return true;
        }

        bool setFrameSize(FrameSize size) override {
            if (!_initialized) {
                return false;
            }

            sensor_t* s = esp_camera_sensor_get();
            if (s) {
                s->set_framesize(s, toEspFrameSize(size));
                return true;
            }
            return false;
        }

        bool setFrameFormat(FrameFormat format) override {
            if (!_initialized) {
                return false;
            }

            sensor_t* s = esp_camera_sensor_get();
            if (s) {
                s->set_pixformat(s, toEspPixelFormat(format));
                return true;
            }
            return false;
        }

        bool setJPEGQuality(uint8_t quality) override {
            if (!_initialized) {
                return false;
            }

            sensor_t* s = esp_camera_sensor_get();
            if (s) {
                s->set_quality(s, quality);
                return true;
            }
            return false;
        }

        bool setBrightness(int8_t level) override {
            if (!_initialized) {
                return false;
            }

            sensor_t* s = esp_camera_sensor_get();
            if (s) {
                s->set_brightness(s, level);
                return true;
            }
            return false;
        }

        bool setContrast(int8_t level) override {
            if (!_initialized) {
                return false;
            }

            sensor_t* s = esp_camera_sensor_get();
            if (s) {
                s->set_contrast(s, level);
                return true;
            }
            return false;
        }

        bool setSaturation(int8_t level) override {
            if (!_initialized) {
                return false;
            }

            sensor_t* s = esp_camera_sensor_get();
            if (s) {
                s->set_saturation(s, level);
                return true;
            }
            return false;
        }

        bool setSpecialEffect(uint8_t effect) override {
            if (!_initialized) {
                return false;
            }

            sensor_t* s = esp_camera_sensor_get();
            if (s) {
                s->set_special_effect(s, effect);
                return true;
            }
            return false;
        }

        bool enableVerticalFlip(bool enable) override {
            if (!_initialized) {
                return false;
            }

            sensor_t* s = esp_camera_sensor_get();
            if (s) {
                s->set_vflip(s, enable ? 1 : 0);
                return true;
            }
            return false;
        }

        bool enableHorizontalMirror(bool enable) override {
            if (!_initialized) {
                return false;
            }

            sensor_t* s = esp_camera_sensor_get();
            if (s) {
                s->set_hmirror(s, enable ? 1 : 0);
                return true;
            }
            return false;
        }

        const char* getModel() override {
            return "OV2640";
        }

        uint32_t getFrameCount() override {
            return _frameCount;
        }
    };

    inline pixformat_t OV2640Camera::toEspPixelFormat(FrameFormat format) {
        switch (format) {
            case FrameFormat::JPEG: return PIXFORMAT_JPEG;
            case FrameFormat::RGB565: return PIXFORMAT_RGB565;
            case FrameFormat::RGB888: return PIXFORMAT_RGB888;
            case FrameFormat::GRAYSCALE: return PIXFORMAT_GRAYSCALE;
            case FrameFormat::YUV422: return PIXFORMAT_YUV422;
            default: return PIXFORMAT_JPEG;
        }
    }
} // namespace Fractonica

#endif // PLATFORM_OV2640_CAMERA_H
