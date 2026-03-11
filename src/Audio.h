//
// Created by Dmitry Popov on 11.03.2026.
//

#ifndef FRACTONICA_AUDIO_H
#define FRACTONICA_AUDIO_H

#include "sokol_audio.h"

namespace Fractonica {
    class Audio {

        int sampleRate;

    public:

        Audio() = default;
        ~Audio() {
            saudio_shutdown();
        }

        void Initialize(void (*streamCallback)(float* buffer, int num_frames, int num_channels, void* user_data)) {

            saudio_desc desc = {};
            desc.stream_userdata_cb = streamCallback;
            desc.user_data = this;
            saudio_setup(&desc);
            sampleRate = saudio_sample_rate();
        }

        [[nodiscard]] int GetSampleRate() const {
            return sampleRate;
        }

    };
}

#endif //FRACTONICA_AUDIO_H