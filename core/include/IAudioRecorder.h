//
// Created by Dmitry Popov on 13.03.2026.
//
#ifndef FRACTONICA_IRECORDER_H
#define FRACTONICA_IRECORDER_H
#include "IFileHandle.h"

namespace Fractonica
{
    enum RecState
    {
        REC_IDLE,
        REC_RECORDING,
        REC_PAUSED
    };

    class IAudioRecorder
    {
    public:
        virtual ~IAudioRecorder() = default;
        virtual bool begin();
        virtual bool start(const char *filename);
        virtual bool loop(int volumeMultiplier);
        virtual bool stop();
        virtual RecState getState();
    };
}

#endif