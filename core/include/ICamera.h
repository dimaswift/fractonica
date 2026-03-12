#ifndef CORE_ICAMERA_H
#define CORE_ICAMERA_H

#include <stddef.h>
#include <stdint.h>


namespace Fractonica {

    enum class FrameFormat {
        JPEG,
        RGB565,
        RGB888,
        GRAYSCALE,
        YUV422
    };

    /**
     * Camera frame size presets
     */
    enum class FrameSize {
        QQVGA,      // 160x120
        QCIF,       // 176x144
        HQVGA,      // 240x176
        QVGA,       // 320x240
        CIF,        // 400x296
        VGA,        // 640x480
        SVGA,       // 800x600
        XGA,        // 1024x768
        SXGA,       // 1280x1024
        UXGA        // 1600x1200
    };

    /**
     * Camera configuration
     */
    struct CameraConfig {
        FrameFormat format;
        FrameSize frameSize;
        uint8_t jpegQuality;    // 0-63, lower means higher quality
        uint8_t frameBuffers;   // Number of frame buffers (1-2)
    };

    /**
     * Camera frame buffer
     */
    struct FrameBuffer {
        uint8_t* buffer;
        size_t length;
        uint32_t width;
        uint32_t height;
        FrameFormat format;
        uint64_t timestamp;     // Timestamp in milliseconds
    };

    /**
     * Camera status information
     */
    struct CameraStatus {
        bool initialized;
        bool ready;
        uint32_t frameCount;
        uint32_t errorCount;
        char model[32];
    };

    /**
     * Abstract interface for camera operations
     * Pure C++ with no platform dependencies
     */
    class ICamera {
    public:
        virtual ~ICamera() {}

        // Initialization
        virtual bool begin() = 0;
        virtual bool begin(const CameraConfig& config) = 0;
        virtual void end() = 0;
        virtual bool isReady() = 0;
        virtual CameraStatus getStatus() = 0;

        // Frame capture
        virtual bool captureFrame(FrameBuffer* frame) = 0;
        virtual bool releaseFrame(FrameBuffer* frame) = 0;

        // Configuration
        virtual bool setFrameSize(FrameSize size) = 0;
        virtual bool setFrameFormat(FrameFormat format) = 0;
        virtual bool setJPEGQuality(uint8_t quality) = 0;

        // Image adjustments
        virtual bool setBrightness(int8_t level) = 0;      // -2 to +2
        virtual bool setContrast(int8_t level) = 0;        // -2 to +2
        virtual bool setSaturation(int8_t level) = 0;      // -2 to +2
        virtual bool setSpecialEffect(uint8_t effect) = 0; // 0=none, 1=negative, 2=grayscale, etc.

        // Control
        virtual bool enableVerticalFlip(bool enable) = 0;
        virtual bool enableHorizontalMirror(bool enable) = 0;

        // Metadata
        virtual const char* getModel() = 0;
        virtual uint32_t getFrameCount() = 0;
    };

} // namespace Fractonica

#endif // CORE_ICAMERA_H
