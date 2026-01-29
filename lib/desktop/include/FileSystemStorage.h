#ifndef PLATFORM_DESKTOP_FILESYSTEMSTORAGE_H
#define PLATFORM_DESKTOP_FILESYSTEMSTORAGE_H

#include <IMemoryStorage.h>
#include <iostream>
#include <fstream>
#include <filesystem>

namespace Fractonica {
    class FileSystemStorage : public IMemoryStorage {
    private:
        bool _initialized;
        std::filesystem::path _basePath;
        std::filesystem::path getFullPath(const char *path) const;

    public:
        explicit FileSystemStorage(const char *basePath = "./fractonica_storage");
        ~FileSystemStorage() override;
        bool begin() override;
        void end() override ;
        bool isReady() const override;
        StorageInfo getInfo() const override;
        bool read(const char *path, char *buffer, size_t maxLen, size_t *bytesRead) const override;
        bool write(const char* path, const uint8_t* buffer, size_t len) override;
        bool append(const char* path, const uint8_t* buffer, size_t len) override;
        bool exists(const char *path) const override;
        bool remove(const char* path) override;
        bool rename(const char* pathFrom, const char* pathTo) override;
        bool mkdir(const char* path) override;
        bool rmdir(const char* path) override;
        bool list(const char *path, FileInfo *files, size_t maxFiles, size_t *fileCount) const override;
        const char *getType() override;
    };
} // namespace Fractonica

#endif // PLATFORM_DESKTOP_FILESYSTEMSTORAGE_H
