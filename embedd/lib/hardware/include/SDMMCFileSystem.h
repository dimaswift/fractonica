#ifndef HARDWARE_SDMMCFILESYSTEM_H
#define HARDWARE_SDMMCFILESYSTEM_H
#include <SD_MMC.h>
#include <FS.h>
#include "IFileHandle.h"
#include "SDMMCStorage.h"

namespace Fractonica {
    class SDMMCFileHandle : public IFileHandle {
    private:
        File file;

    public:
        SDMMCFileHandle(File f) : file(f) {
        }

        bool isOpen() const override {
            return file;
        }

        void close() override {
            if (file) file.close();
        }

        size_t write(const uint8_t *data, size_t size) override {
            if (!file) return 0;
            return file.write(data, size);
        }

        size_t read(uint8_t *buffer, size_t size) override {
            if (!file) return 0;
            return file.read(buffer, size);
        }

        bool seek(long offset, SeekOrigin origin) override {
            if (!file) return false;

            SeekMode mode;
            switch (origin) {
                case SeekOrigin::Begin: mode = SeekSet;
                    break;
                case SeekOrigin::Current: mode = SeekCur;
                    break;
                case SeekOrigin::End: mode = SeekEnd;
                    break;
                default: return false;
            }

            return file.seek(offset, mode);
        }

        long tell() const override {
            if (!file) return -1;
            return file.position();
        }

        bool flush() override {
            if (!file) return false;
            file.flush();
            return true;
        }

        long size() const override {
            if (!file) return -1;
            return file.size();
        }
    };

    // ─────────────────────────────────────────────

    class SDMMCFileSystem : public IFileSystem {
    private:
        bool mounted = false;
        SDCardPins _pins;

    public:
        SDMMCFileSystem(const SDCardPins &pins = {}) : _pins(pins){

        }

        bool begin() override {
            SD_MMC.setPins(_pins.clk, _pins.cmd, _pins.data0); // CLK, CMD, DATA0
            mounted = SD_MMC.begin("/sdcard", _pins.mode1Bit);
            return mounted;
        }

        IFileHandle *openRead(const char *path) override {
            if (!mounted) return nullptr;
            File f = SD_MMC.open(path, FILE_READ);
            if (!f) return nullptr;
            return new SDMMCFileHandle(f);
        }

        IFileHandle *openWrite(const char *path) override {
            if (!mounted) return nullptr;
            File f = SD_MMC.open(path, FILE_WRITE); // truncates or creates
            if (!f) return nullptr;
            return new SDMMCFileHandle(f);
        }

        IFileHandle *openAppend(const char *path) override {
            if (!mounted) return nullptr;
            File f = SD_MMC.open(path, FILE_APPEND);
            if (!f) return nullptr;
            return new SDMMCFileHandle(f);
        }

        bool exists(const char *path) const override {
            if (!mounted) return false;
            return SD_MMC.exists(path);
        }

        bool mkdir(const char *path) override {
            if (!mounted) return false;
            if (SD_MMC.exists(path)) return true;
            if (path[0] != '/') {
                char fullPath[128];
                sprintf(fullPath, "/%s", path);
                return SD_MMC.mkdir(fullPath);
            }
            return SD_MMC.mkdir(path);
        }

        bool remove(const char *path) override {
            if (!mounted) return false;
            return SD_MMC.remove(path);
        }

        uint64_t totalBytes() const override {
            if (!mounted) return 0;
            return SD_MMC.totalBytes();
        }

        uint64_t usedBytes() const override {
            if (!mounted) return 0;
            return SD_MMC.usedBytes();
        }

        uint64_t freeBytes() const override {
            if (!mounted) return 0;
            return totalBytes() - usedBytes();
        }
    };
}
#endif
