#ifndef IMEMORYSTORAGE_H
#define IMEMORYSTORAGE_H

#include <stddef.h>
#include <stdint.h>
#include <time.h>

namespace Fractonica {

    /**
     * File information structure
     */
    struct FileInfo {
        char name[256];
        size_t size;
        bool isDirectory;
        time_t lastModified;
    };

    /**
     * Storage statistics
     */
    struct StorageInfo {
        uint64_t totalBytes;
        uint64_t usedBytes;
        uint64_t freeBytes;
        char type[32];
    };

    /**
     * Abstract interface for memory storage operations
     * Pure C++ with no platform dependencies
     */
    class IMemoryStorage {
    public:
        virtual ~IMemoryStorage() {}

        // Initialization
        virtual bool begin() = 0;
        virtual void end() = 0;
        virtual bool isReady() const = 0;
        virtual StorageInfo getInfo() const = 0;

        // File operations
        virtual bool read(const char *path, char *buffer, size_t maxLen, size_t *bytesRead) const = 0;
        virtual bool write(const char* path, const uint8_t* buffer, size_t len) = 0;
        virtual bool append(const char* path, const uint8_t* buffer, size_t len) = 0;

        // File management
        virtual bool exists(const char *path) const = 0;
        virtual bool remove(const char* path) = 0;
        virtual bool rename(const char* pathFrom, const char* pathTo) = 0;

        // Directory operations
        virtual bool mkdir(const char* path) = 0;
        virtual bool rmdir(const char* path) = 0;
        virtual bool list(const char *path, FileInfo *files, size_t maxFiles, size_t *fileCount) const = 0;

        // Metadata
        virtual const char* getType() = 0;
    };

} // namespace Fractonica

#endif // CORE_IMEMORYSTORAGE_H
