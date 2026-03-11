//
// Created by Dmitry Popov on 10.03.2026.
//

#ifndef EMBEDD_SYNTH_H
#define EMBEDD_SYNTH_H

#include <Arduino.h>
#include <driver/i2s.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

// Type definition for your custom procedural audio function.
// It should return a 16-bit PCM audio sample.
typedef int16_t (*AudioGeneratorFunc)();

namespace Fractonica {
    class I2SAudio {
    public:
        I2SAudio(i2s_port_t port = I2S_NUM_0)
            : m_i2s_port(port), m_audio_task_handle(nullptr), m_generator(nullptr) {
        }

        ~I2SAudio() {
            stop();
        }

        // Initialize the I2S hardware and start the Core 0 audio task
        bool begin(int bck_pin, int ws_pin, int data_out_pin, int sample_rate, AudioGeneratorFunc generator) {
            m_generator = generator;
            sampleRate = sample_rate;

            // 1. Configure the I2S peripheral
            i2s_config_t i2s_config = {
                .mode = (i2s_mode_t) (I2S_MODE_MASTER | I2S_MODE_TX),
                .sample_rate = (uint32_t) sample_rate,
                .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
                .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT, // Mono synth
                .communication_format = I2S_COMM_FORMAT_STAND_I2S,
                .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
                .dma_buf_count = 4,
                .dma_buf_len = 512,
                .use_apll = true, // Use high-precision audio clock on ESP32
                .tx_desc_auto_clear = true
            };

            i2s_pin_config_t pin_config = {
                .bck_io_num = bck_pin, // Bit Clock (BCLK)
                .ws_io_num = ws_pin, // Word Select (LRC)
                .data_out_num = data_out_pin, // Data Out (DIN)
                .data_in_num = I2S_PIN_NO_CHANGE
            };

            if (i2s_driver_install(m_i2s_port, &i2s_config, 0, NULL) != ESP_OK) return false;
            if (i2s_set_pin(m_i2s_port, &pin_config) != ESP_OK) return false;

            // 2. Spawn the FreeRTOS task pinned to Core 0
            // Parameters: Task function, Name, Stack size, Context pointer, Priority, Handle, Core ID
            xTaskCreatePinnedToCore(
                I2SAudio::audioTaskTrampoline,
                "SynthAudioTask",
                4096,
                this,
                configMAX_PRIORITIES - 1, // High priority for audio
                &m_audio_task_handle,
                0 // Pin to Core 0 (PRO_CPU)
            );

            return true;
        }

        void stop() {
            if (m_audio_task_handle != nullptr) {
                vTaskDelete(m_audio_task_handle);
                m_audio_task_handle = nullptr;
            }
            i2s_driver_uninstall(m_i2s_port);
        }

    private:
        i2s_port_t m_i2s_port;
        TaskHandle_t m_audio_task_handle;
        AudioGeneratorFunc m_generator;
        int sampleRate;

        // FreeRTOS requires a static function for tasks. We use this trampoline
        // to jump back into our class instance.
        static void audioTaskTrampoline(void *arg) {
            I2SAudio *instance = static_cast<I2SAudio *>(arg);
            instance->audioTask();
        }

        // The actual loop running permanently on Core 0
        void audioTask() {
            const int BUFFER_SAMPLES = 256;
            int16_t sample_buffer[BUFFER_SAMPLES];
            size_t bytes_written;

            while (true) {
                // 1. Generate a chunk of audio
                for (int i = 0; i < BUFFER_SAMPLES; i++) {
                    if (m_generator) {
                        sample_buffer[i] = m_generator();
                    } else {
                        sample_buffer[i] = 0;
                    }
                }

                // 2. Push it to the I2S DMA buffer.
                // portMAX_DELAY makes this thread sleep seamlessly until the hardware needs more data.
                i2s_write(m_i2s_port, sample_buffer, sizeof(sample_buffer), &bytes_written, portMAX_DELAY);
            }
        }
    };
}

#endif //EMBEDD_SYNTH_H
