
#include "FileSystemStorage.h"

namespace Fractonica
{
    /**
     * Constructor
     * @param basePath Base directory for simulated storage (default: ./fractonica_storage)
     */
    FileSystemStorage::FileSystemStorage(const char *basePath)
        : _initialized(false), _basePath(basePath)
    {
    }

    FileSystemStorage::~FileSystemStorage()
    {
        FileSystemStorage::end();
    }

    std::filesystem::path FileSystemStorage::getFullPath(const char *path) const
    {
        if (!path || !*path)
            return _basePath;
        std::string_view sv(path);
        if (!sv.empty() && sv.front() == '/')
            sv.remove_prefix(1);
        return _basePath / std::filesystem::path(sv);
    }

    bool FileSystemStorage::begin()
    {
        if (_initialized)
            return true;

        try
        {
            // Create base directory if it doesn't exist
            if (!std::filesystem::exists(_basePath))
            {
                std::filesystem::create_directories(_basePath);
            }
            _initialized = true;
            std::cout << "  Desktop storage initialized at: " << _basePath << std::endl;
            return true;
        }
        catch (const std::exception &e)
        {
            std::cerr << "  Failed to initialize desktop storage: " << e.what() << std::endl;
            return false;
        }
    }

    void FileSystemStorage::end()
    {
        _initialized = false;
    }

    bool FileSystemStorage::isReady() const
    {
        return _initialized;
    }

    StorageInfo FileSystemStorage::getInfo() const
    {
        StorageInfo info{};
        strncpy(info.type, "FileSystem", sizeof(info.type) - 1);

        if (!_initialized)
        {
            info.totalBytes = 0;
            info.usedBytes = 0;
            info.freeBytes = 0;
            return info;
        }

        try
        {
            auto spaceInfo = std::filesystem::space(_basePath);
            info.totalBytes = spaceInfo.capacity;
            info.freeBytes = spaceInfo.available;
            info.usedBytes = info.totalBytes - info.freeBytes;
        }
        catch (...)
        {
            info.totalBytes = 0;
            info.usedBytes = 0;
            info.freeBytes = 0;
        }

        return info;
    }

    bool FileSystemStorage::read(const char *path, char *buffer, size_t maxLen, size_t *bytesRead) const
    {
        if (!_initialized || !buffer)
            return false;

        try
        {
            auto fullPath = getFullPath(path);
            std::ifstream file(fullPath, std::ios::binary);
            if (!file.is_open())
                return false;

            file.read(buffer, maxLen);
            *bytesRead = file.gcount();
            file.close();
            return true;
        }
        catch (...)
        {
            return false;
        }
    }

    bool FileSystemStorage::write(const char *path, const uint8_t *buffer, size_t len)
    {
        if (!_initialized || !buffer)
            return false;

        try
        {
            auto fullPath = getFullPath(path);
            // Create parent directories if needed
            std::filesystem::create_directories(fullPath.parent_path());

            std::ofstream file(fullPath, std::ios::binary | std::ios::trunc);
            if (!file.is_open())
                return false;

            file.write(reinterpret_cast<const char *>(buffer), len);
            file.close();
            return true;
        }
        catch (...)
        {
            return false;
        }
    }

    bool FileSystemStorage::append(const char *path, const uint8_t *buffer, size_t len)
    {
        if (!_initialized || !buffer)
            return false;

        try
        {
            auto fullPath = getFullPath(path);
            std::ofstream file(fullPath, std::ios::binary | std::ios::app);
            if (!file.is_open())
                return false;

            file.write(reinterpret_cast<const char *>(buffer), len);
            file.close();
            return true;
        }
        catch (...)
        {
            return false;
        }
    }

    bool FileSystemStorage::exists(const char *path) const
    {
        if (!_initialized)
            return false;
        return std::filesystem::exists(getFullPath(path));
    }

    bool FileSystemStorage::remove(const char *path)
    {
        if (!_initialized)
            return false;
        try
        {
            return std::filesystem::remove(getFullPath(path));
        }
        catch (...)
        {
            return false;
        }
    }

    bool FileSystemStorage::rename(const char *pathFrom, const char *pathTo)
    {
        if (!_initialized)
            return false;
        try
        {
            std::filesystem::rename(getFullPath(pathFrom), getFullPath(pathTo));
            return true;
        }
        catch (...)
        {
            return false;
        }
    }

    bool FileSystemStorage::mkdir(const char *path)
    {
        if (!_initialized)
            return false;
        try
        {
            return std::filesystem::create_directories(getFullPath(path));
        }
        catch (...)
        {
            return false;
        }
    }

    bool FileSystemStorage::rmdir(const char *path)
    {
        if (!_initialized)
            return false;
        try
        {
            return std::filesystem::remove_all(getFullPath(path));
        }
        catch (...)
        {
            return false;
        }
    }

    bool FileSystemStorage::list(const char *path, FileInfo *files, size_t maxFiles, size_t *fileCount) const
    {
        if (!_initialized)
            return false;

        try
        {
            auto fullPath = getFullPath(path);
            *fileCount = 0;

            for (const auto &entry : std::filesystem::directory_iterator(fullPath))
            {
                if (*fileCount >= maxFiles)
                    break;

                std::string filename = entry.path().filename().string();
                strncpy(files[*fileCount].name, filename.c_str(), sizeof(files[*fileCount].name) - 1);
                files[*fileCount].name[sizeof(files[*fileCount].name) - 1] = '\0';

                files[*fileCount].isDirectory = entry.is_directory();
                files[*fileCount].size = entry.is_regular_file() ? entry.file_size() : 0;

                // Get last modified time
                auto ftime = std::filesystem::last_write_time(entry);
                auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                    ftime - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now());
                files[*fileCount].lastModified = std::chrono::system_clock::to_time_t(sctp);

                (*fileCount)++;
            }
            return true;
        }
        catch (...)
        {
            *fileCount = 0;
            return false;
        }
    }

    const char *FileSystemStorage::getType()
    {
        return "FileSystem";
    }
};

