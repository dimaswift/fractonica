//
// Created by Dmitry Popov on 13.03.2026.
//
#ifndef FRACTONICA_EVENTLOGGER_H
#define FRACTONICA_EVENTLOGGER_H


#include <algorithm>

#include "IAudioRecorder.h"
#include "ICamera.h"
#include "IFileHandle.h"
#include "ITime.h"
#include "IUnixClock.h"
#include "saros.h"
#include "Utils.h"

namespace Fractonica {

    class EventLogger {

    private:

        #define NAME_SIZE 3

        int sarosNumber;
        IUnixClock* unixClock;
        IFileSystem* fileSystem;

        IAudioRecorder* audioRecorder;

        FrameBuffer frameBuffer = {};

        char currentLogRoot[32];
        char currentFramesRoot[64];
        bool isRecording = false;
        uint64_t startTime = 0;
        uint64_t currentBin = 0;
        uint16_t frameCounter = 0;

    public:

        EventLogger(int saros,IUnixClock* clock, IFileSystem* fs,  IAudioRecorder* rec) : sarosNumber(
           saros), unixClock(clock), fileSystem(fs), audioRecorder(rec), currentLogRoot{},
       currentFramesRoot{} {
        }

        bool recording() const {
            return isRecording;
        }

        bool captureFrame() {

            if (!isRecording) {
                return false;
            }


            return false;
        }

        bool startLog() {

            if (isRecording) {
                return false;
            }
            startTime = unixClock->now();
            uint64_t bin = calculate_solar_octal_phase(startTime, sarosNumber, NAME_SIZE);
            currentBin = Utils::DecimaToOctal(bin);
            snprintf(currentLogRoot, sizeof(currentLogRoot), "/%lld", currentBin);
            frameCounter = 0;

            if (!fileSystem->mkdir(currentLogRoot)) {
                return false;
            }

            isRecording = true;

            captureFrame();

            if (audioRecorder && audioRecorder->getState() == RecState::REC_IDLE) {

                char recPath[32];
                snprintf(recPath, sizeof(recPath), "%s/rec.wav", currentLogRoot);
                audioRecorder->start(recPath);
            }

            return true;
        }

        bool endLog() {

            isRecording = false;
            if (audioRecorder && audioRecorder->getState() == RecState::REC_RECORDING) {
                audioRecorder->stop();

            }

            char infoPath[32];
            snprintf(infoPath, sizeof(infoPath), "%s/info.json", currentLogRoot);

            char info[256];
            int len = snprintf(info, sizeof(info), "{\"saros\": %d, \"bin\": %lld, \"start\": %lld, \"duration\": %lld, \"frames\": %d}",
                sarosNumber, currentBin, startTime, unixClock->now() - startTime, frameCounter);

            if (len == 0) {
                return false;
            }

            IFileHandle* h = fileSystem->openWrite(infoPath);
            if (!h) {
                return false;
            }
            auto written = h->write((uint8_t*)info, len);
            h->flush();
            h->close();
            delete h;
            return written == len;
        }

    };
}


#endif //FRACTONICA_EVENTLOGGER_H