#ifndef PLATFORM_FRACTONICA_SDMEMORYSTORAGE_H
#define PLATFORM_FRACTONICA_SDMEMORYSTORAGE_H

#include "IMemoryStorage.h"
#include "SD_MMC.h"

namespace Fractonica
{
    struct SDCardPins {
        int clk = 39;
        int cmd = 38;
        int data0 = 40;
        int data1 = -1;  // Optional for 4-bit mode
        int data2 = -1;;  // Optional for 4-bit mode
        int data3 = -1;;  // Optional for 4-bit mode
        bool mode1Bit = true;
    };

    class SDMMCStorage : public IMemoryStorage
    {
    private:
        bool _initialized;
        SDCardPins _pins;

    public:
        /**
         * Constructor using platform config
         */
        SDMMCStorage(const SDCardPins &pins)
            : _initialized(false)
        {
            _pins = pins;
        }

        /**
         * Constructor with explicit pins (override config)
         */
        SDMMCStorage(int clk = 39, int cmd = 38, int data0 = 40, bool mode1Bit = true)
            : _initialized(false)
        {
            _pins.clk = clk;
            _pins.cmd = cmd;
            _pins.data0 = data0;
            _pins.mode1Bit = mode1Bit;
        }

        ~SDMMCStorage() override {
            SDMMCStorage::end();
        }

        bool begin() override
        {
            if (_initialized)
                return true;

            SD_MMC.setPins(_pins.clk, _pins.cmd, _pins.data0); // CLK, CMD, DATA0

            if (!SD_MMC.begin("/sdcard", _pins.mode1Bit))
            {
                return false;
            }

            uint8_t cardType = SD_MMC.cardType();

            if (cardType == CARD_NONE)
            {
                return false;
            }

            _initialized = true;
            return true;
        }

        void end() override
        {
            if (_initialized)
            {
                SD_MMC.end();
                _initialized = false;
            }
        }

        [[nodiscard]] bool isReady() const override
        {
            return _initialized && (SD_MMC.cardType() != CARD_NONE);
        }

        [[nodiscard]] StorageInfo getInfo() const override
        {
            StorageInfo info = {};
            strncpy(info.type, "SD_MMC", sizeof(info.type) - 1);
            info.type[sizeof(info.type) - 1] = '\0'; // ✅ Ensure null termination

            if (!_initialized)
            {
                return info;
            }

            // ✅ Wrap in try-catch or check isReady first
            if (SD_MMC.cardType() == CARD_NONE)
            {
                return info;
            }

            info.totalBytes = SD_MMC.totalBytes();
            info.usedBytes = SD_MMC.usedBytes();
            info.freeBytes = info.totalBytes - info.usedBytes;

            return info;
        }

        bool read(const char *path, char *buffer, size_t maxLen, size_t *bytesRead) const override
        {
            if (!_initialized || !buffer || !bytesRead || maxLen == 0)
            {
                if (bytesRead)
                    *bytesRead = 0;
                return false;
            }

            File file = SD_MMC.open(path, FILE_READ);
            if (!file)
            {
                *bytesRead = 0;
                return false;
            }

            if (file.isDirectory())
            {
                file.close();
                *bytesRead = 0;
                return false;
            }

            size_t fileSize = file.size();
            size_t toRead = (fileSize < maxLen) ? fileSize : maxLen;

            *bytesRead = file.read((uint8_t *)buffer, toRead);
            file.close();

            if (*bytesRead > maxLen)
            {
                Serial.printf("[ERROR] Read overflow! bytesRead=%zu, maxLen=%zu\n", *bytesRead, maxLen);
                *bytesRead = maxLen;
            }

            return (*bytesRead > 0);
        }

        bool write(const char *path, const uint8_t *buffer, size_t len) override
        {
            if (!_initialized || !buffer)
                return false;

            File file = SD_MMC.open(path, FILE_WRITE);
            if (!file)
                return false;

            size_t written = file.write(buffer, len);
            file.close();

            return written == len;
        }

        File openForWrite(String filename) {
            return SD_MMC.open(filename, FILE_WRITE);
        }

        File openForRead(String filename) {
            return SD_MMC.open(filename, FILE_READ);
        }

        bool append(const char *path, const uint8_t *buffer, size_t len) override
        {
            if (!_initialized || !buffer)
                return false;

            File file = SD_MMC.open(path, FILE_APPEND);
            if (!file)
                return false;

            size_t written = file.write(buffer, len);
            file.close();

            return written == len;
        }

        bool exists(const char *path) const
        {
            if (!_initialized)
                return false;
            return SD_MMC.exists(path);
        }

        bool remove(const char *path) override
        {
            if (!_initialized)
                return false;
            return SD_MMC.remove(path);
        }

        bool remove(String path)
        {
            if (!_initialized)
                return false;
            return SD_MMC.remove(path);
        }

        bool rename(const char *pathFrom, const char *pathTo) override
        {
            if (!_initialized)
                return false;
            return SD_MMC.rename(pathFrom, pathTo);
        }

        bool mkdir(const char *path) override
        {
            if (!_initialized)
                return false;
            return SD_MMC.mkdir(path);
        }

        bool rmdir(const char *path) override
        {
            if (!_initialized)
                return false;
            return SD_MMC.rmdir(path);
        }

        bool list(const char *path, FileInfo *files, size_t maxFiles, size_t *fileCount) const
        {
            if (!_initialized || !path || !files || !fileCount || maxFiles == 0)
            {
                if (fileCount)
                    *fileCount = 0;
                return false;
            }

            *fileCount = 0;

            File dir = SD_MMC.open(path);
            if (!dir)
                return false;
            if (!dir.isDirectory())
            {
                dir.close();
                return false;
            }

            while (*fileCount < maxFiles)
            {
                File entry = dir.openNextFile();
                if (!entry)
                    break;

                FileInfo &out = files[*fileCount];
                memset(&out, 0, sizeof(out));

                const char *name = entry.name();
                if (name)
                {
                    strncpy(out.name, name, sizeof(out.name) - 1);
                    out.name[sizeof(out.name) - 1] = '\0';
                }

                out.isDirectory = entry.isDirectory();
                out.size = entry.size();
                out.lastModified = entry.getLastWrite();

                entry.close();
                (*fileCount)++;
            }

            dir.close();
            return true;
        }

        const char *getType() override
        {
            return "SD_MMC";
        }

    };

} // namespace Fractonica

#endif // PLATFORM_FRACTONICA_SDMEMORYSTORAGE_H
