#ifndef FRACTONICA_IFILEHANDLE_H
#define FRACTONICA_IFILEHANDLE_H

#include <stddef.h>
#include <stdint.h>

namespace Fractonica {

    /**
     * Seek origin for file operations
     */
    enum class SeekOrigin {
        Begin,      // Seek from beginning of file
        Current,    // Seek from current position
        End         // Seek from end of file
    };

    /**
     * Abstract interface for file handle operations
     * Provides low-level file I/O without platform dependencies
     */
    class IFileHandle {
    public:
        virtual ~IFileHandle() = default;

        /**
         * Check if the file handle is valid/open
         */
        virtual bool isOpen() const = 0;

        /**
         * Close the file handle
         */
        virtual void close() = 0;

        /**
         * Write data to file
         * @param data Pointer to data buffer
         * @param size Number of bytes to write
         * @return Number of bytes actually written
         */
        virtual size_t write(const uint8_t* data, size_t size) = 0;

        /**
         * Read data from file
         * @param buffer Pointer to destination buffer
         * @param size Maximum number of bytes to read
         * @return Number of bytes actually read
         */
        virtual size_t read(uint8_t* buffer, size_t size) = 0;

        /**
         * Seek to a position in the file
         * @param offset Offset from origin
         * @param origin Seek origin
         * @return true if successful
         */
        virtual bool seek(long offset, SeekOrigin origin) = 0;

        /**
         * Get current file position
         * @return Current position in bytes from start of file
         */
        virtual long tell() const = 0;

        /**
         * Flush any buffered data to storage
         * @return true if successful
         */
        virtual bool flush() = 0;

        /**
         * Get the size of the file
         * @return File size in bytes, or -1 on error
         */
        virtual long size() const = 0;
    };

    /**
     * Factory interface for creating file handles
     * Abstracts away filesystem-specific file opening
     */
    class IFileSystem {
    public:
        virtual ~IFileSystem() = default;
        virtual bool begin() = 0;
        /**
         * Open a file for reading
         * @param path File path
         * @return File handle, or nullptr on failure. Caller owns the returned pointer.
         */
        virtual IFileHandle* openRead(const char* path) = 0;

        /**
         * Open a file for writing (creates or truncates)
         * @param path File path
         * @return File handle, or nullptr on failure. Caller owns the returned pointer.
         */
        virtual IFileHandle* openWrite(const char* path) = 0;

        /**
         * Open a file for appending
         * @param path File path
         * @return File handle, or nullptr on failure. Caller owns the returned pointer.
         */
        virtual IFileHandle* openAppend(const char* path) = 0;

        /**
         * Check if a file or directory exists
         * @param path Path to check
         * @return true if exists
         */
        virtual bool exists(const char* path) const = 0;

        /**
         * Create a directory
         * @param path Directory path
         * @return true if successful or already exists
         */
        virtual bool mkdir(const char* path) = 0;

        /**
         * Remove a file
         * @param path File path
         * @return true if successful
         */
        virtual bool remove(const char* path) = 0;

        /**
         * Get total storage capacity in bytes
         */
        virtual uint64_t totalBytes() const = 0;

        /**
         * Get used storage in bytes
         */
        virtual uint64_t usedBytes() const = 0;

        /**
         * Get free storage in bytes
         */
        virtual uint64_t freeBytes() const = 0;
    };

} // namespace Fractonica

#endif // FRACTONICA_IFILEHANDLE_H
