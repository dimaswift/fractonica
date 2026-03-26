#ifndef AUDIO_RECORDER_H
#define AUDIO_RECORDER_H
#define CONFIG_I2S_SUPPRESS_DEPRECATE_WARN 1

#include <Arduino.h>
#include <driver/i2s.h>
#include "IFileHandle.h"
#include "IAudioRecorder.h"
#include <cstring>

namespace Fractonica
{

#define I2S_PORT I2S_NUM_1
#define I2S_SAMPLE_RATE 16000
#define I2S_BUFFER_SIZE 512 // int16 samples
#define AUDIO_BITS 16

#pragma pack(push, 1)
    typedef struct
    {
        char riff[4];
        uint32_t overall_size;
        char wave[4];
        char fmt_chunk_marker[4];
        uint32_t length_of_fmt;
        uint16_t format_type;
        uint16_t channels;
        uint32_t sample_rate;
        uint32_t byterate;
        uint16_t block_align;
        uint16_t bits_per_sample;
        char data_chunk_header[4];
        uint32_t data_size;
    } WavHeader;
#pragma pack(pop)

    class I2SRecorder : public IAudioRecorder
    {
    private:
        IFileSystem *_storage;
        RecState _state;
        IFileHandle* _file;
        const char *_currentFilename;

        int16_t _i2sBuf[I2S_BUFFER_SIZE];
        int _ws, _sck, _sd;

        size_t _currentByteCount = 0;

        // PSRAM chunk buffer
        uint8_t *_chunkBuf = nullptr;
        size_t _chunkSize = 0; // bytes
        size_t _chunkUsed = 0; // bytes

        bool flushChunkToSD()
        {
            if (!_file || _chunkUsed == 0)
                return true;

            size_t w = _file->write(_chunkBuf, _chunkUsed);
            if (w != _chunkUsed)
            {
                _file->close();
                _state = REC_IDLE;
                _storage->remove(_currentFilename);
                return false;
            }
            _currentByteCount += _chunkUsed;
            _chunkUsed = 0;

            _file->flush();
            return true;
        }

    public:
        // chunkSizeBytes e.g. 2*1024*1024 for 2 MiB
        I2SRecorder(IFileSystem *storage, int ws, int sck, int sd, size_t chunkSizeBytes = 2 * 1024 * 1024)
            : _storage(storage), _ws(ws), _sck(sck), _sd(sd), _state(REC_IDLE), _file(nullptr),
              _currentFilename(nullptr), _i2sBuf{},
              _chunkSize(chunkSizeBytes) {
        }

        bool begin() override
        {
            // Allocate PSRAM buffer once
            if (_chunkBuf == nullptr)
            {
                _chunkBuf = (uint8_t *)ps_malloc(_chunkSize);
                if (!_chunkBuf)
                {
                    // Fallback: try regular heap (less ideal)
                    _chunkBuf = (uint8_t *)malloc(_chunkSize);
                    if (!_chunkBuf)
                        return false;
                }
            }

            i2s_config_t i2s_config = {
                .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
                .sample_rate = I2S_SAMPLE_RATE,
                .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
                .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
                .communication_format = I2S_COMM_FORMAT_STAND_I2S,
                .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
                .dma_desc_num = 8,
                .dma_frame_num = I2S_BUFFER_SIZE,
                .use_apll = false,
                .tx_desc_auto_clear = false,
                .fixed_mclk = 0};

            if (i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL) != ESP_OK)
                return false;

            i2s_pin_config_t pin_config = {
                .mck_io_num = I2S_PIN_NO_CHANGE,
                .bck_io_num = _sck,
                .ws_io_num = _ws,
                .data_out_num = I2S_PIN_NO_CHANGE,
                .data_in_num = _sd};

            if (i2s_set_pin(I2S_PORT, &pin_config) != ESP_OK)
                return false;

            // Ensure DMA starts clean
            i2s_zero_dma_buffer(I2S_PORT);
            return true;
        }

        size_t getFileSize() const { return _currentByteCount; }

        bool start(const char *path) override
        {
            if (_state != REC_IDLE)
                return false;

            _currentByteCount = 0;
            _chunkUsed = 0;
            _currentFilename = path;

            _file = _storage->openWrite(_currentFilename);
            if (!_file)
                return false;

            // Write placeholder WAV header (zeros)
            WavHeader header{};
            size_t hw = _file->write((const uint8_t *)&header, sizeof(header));
            if (hw != sizeof(header))
            {
                _file->close();
                _storage->remove(_currentFilename);
                return false;
            }
            _file->flush();

            _state = REC_RECORDING;
            return true;
        }

        bool loop(int volumeMultiplier) override
        {
            if (_state != REC_RECORDING)
                return false;

            size_t bytes_read = 0;
            esp_err_t result = i2s_read(I2S_PORT, _i2sBuf, sizeof(_i2sBuf), &bytes_read, 10);

            if (result != ESP_OK || bytes_read == 0)
                return false;

            // Apply gain in-place (16-bit PCM)
            if (volumeMultiplier != 1)
            {
                int16_t *samples = (int16_t *)_i2sBuf;
                int sampleCount = bytes_read / 2;
                for (int i = 0; i < sampleCount; i++)
                {
                    int32_t amplified = (int32_t)samples[i] * volumeMultiplier;
                    if (amplified > 32767)
                        amplified = 32767;
                    if (amplified < -32768)
                        amplified = -32768;
                    samples[i] = (int16_t)amplified;
                }
            }

            // Append into PSRAM chunk buffer; if full, flush chunk to SD first.
            const uint8_t *src = (const uint8_t *)_i2sBuf;
            size_t remaining = bytes_read;

            while (remaining > 0)
            {
                size_t space = _chunkSize - _chunkUsed;
                if (space == 0)
                {
                    if (!flushChunkToSD())
                        return false; // write big chunk
                    space = _chunkSize;
                }

                size_t toCopy = (remaining < space) ? remaining : space;
                memcpy(_chunkBuf + _chunkUsed, src, toCopy);
                _chunkUsed += toCopy;
                src += toCopy;
                remaining -= toCopy;
            }

            return true;
        }

        bool stop() override
        {
            if (_state != REC_RECORDING)
                return false;

            // Write any remaining buffered audio
            if (!flushChunkToSD())
            {
                // flushChunkToSD already closed/removed on failure
                _state = REC_IDLE;
                return false;
            }

            uint32_t fileSize = _file->size();

            WavHeader header{};
            memcpy(header.riff, "RIFF", 4);
            header.overall_size = fileSize - 8;
            memcpy(header.wave, "WAVE", 4);
            memcpy(header.fmt_chunk_marker, "fmt ", 4);
            header.length_of_fmt = 16;
            header.format_type = 1;
            header.channels = 1;
            header.sample_rate = I2S_SAMPLE_RATE;
            header.bits_per_sample = 16;
            header.block_align = 2;
            header.byterate = I2S_SAMPLE_RATE * 2;
            memcpy(header.data_chunk_header, "data", 4);
            header.data_size = fileSize - 44;

            _file->seek(0, SeekOrigin::Begin);
            _file->write((uint8_t *)&header, sizeof(WavHeader));
            _file->flush();
            _file->close();

            _state = REC_IDLE;

            return true;
        }

        RecState getState() override { return _state; }

        ~I2SRecorder() override {
            if (_chunkBuf)
                free(_chunkBuf); // ps_malloc uses free()
            _chunkBuf = nullptr;
        }
    };

} // namespace Fractonica

#endif