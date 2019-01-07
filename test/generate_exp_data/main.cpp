#include <array>
#include <cstdio>
#include <fcntl.h>
#include <unistd.h>
#include "base/os.h"
#include "fs/fs.h"
#include "utils.h"

using namespace services::fs;

extern "C" {
extern void initialise_monitor_handles(void);
extern void __libc_init_array(void);
extern int kill(pid_t, int);
}

class PosixFileSystem : public IFileSystem
{
  public:
    virtual FileOpenResult Open(const char* path, FileOpen openFlag, FileAccess accessMode) override
    {
        auto f = open(path, num(openFlag) | O_BINARY, num(accessMode));

        if (f == -1)
        {
            return FileOpenResult(static_cast<OSResult>(errno), -1);
        }
        else
        {
            return FileOpenResult(OSResult::Success, f);
        }
    }

    virtual OSResult Unlink(const char* /*path*/) override
    {
        return OSResult::NotSupported;
    }

    virtual OSResult Move(const char* /*from*/, const char* /*to*/) override
    {
        return OSResult::NotSupported;
    }

    virtual OSResult Copy(const char* /*from*/, const char* /*to*/) override
    {
        return OSResult::NotSupported;
    }

    virtual OSResult TruncateFile(FileHandle /*file*/, FileSize /*length*/) override
    {
        return OSResult::NotSupported;
    }

    virtual IOResult Write(FileHandle file, gsl::span<const std::uint8_t> buffer) override
    {
        auto r = write(file, buffer.data(), buffer.size());

        if (r == -1)
        {
            return IOResult(static_cast<OSResult>(errno), {});
        }
        else
        {
            return IOResult(OSResult::Success, buffer.subspan(0, r));
        }
    }

    virtual IOResult Read(FileHandle file, gsl::span<std::uint8_t> buffer) override
    {
        auto r = read(file, buffer.data(), buffer.size());

        if (r == -1)
        {
            return IOResult(static_cast<OSResult>(errno), {});
        }
        else
        {
            return IOResult(OSResult::Success, buffer.subspan(0, r));
        }
    }

    virtual OSResult Close(FileHandle file) override
    {
        return static_cast<OSResult>(close(file));
    }

    virtual DirectoryOpenResult OpenDirectory(const char* /*dirname*/) override
    {
        return DirectoryOpenResult(OSResult::NotSupported, 0);
    }

    virtual char* ReadDirectory(DirectoryHandle /*directory*/) override
    {
        return nullptr;
    }

    virtual OSResult CloseDirectory(DirectoryHandle /*directory*/) override
    {
        return OSResult::NotSupported;
    }

    virtual bool IsDirectory(const char* /*dirname*/) override
    {
        return false;
    }

    virtual OSResult Format(const char* /*mountPoint*/) override
    {
        return OSResult::NotSupported;
    }

    virtual OSResult MakeDirectory(const char* /*path*/) override
    {
        return OSResult::NotSupported;
    }

    virtual bool Exists(const char* /*path*/) override
    {
        return false;
    }

    virtual FileSize GetFileSize(FileHandle /*file*/) override
    {
        return 0;
    }

    virtual FileSize GetFileSize(const char* /*dir*/, const char* /*file*/) override
    {
        return 0;
    }

    virtual OSResult Seek(FileHandle file, SeekOrigin origin, FileSize offset) override
    {
        return static_cast<OSResult>(lseek(file, offset, num(origin)));
    }

    virtual std::uint32_t GetFreeSpace(const char* /*devicePath*/) override
    {
        return 0;
    }
};

void GenerateSunSData(IFileSystem& fs);
void GenerateSailData(IFileSystem& fs);
void GenerateCameraCommissioningData(IFileSystem& fs);
void GeneratePayloadCommissioningData(IFileSystem& fs);
void GenerateLeopData(IFileSystem& fs);
void GenerateRadfetData(IFileSystem& fs);

int main()
{
    __libc_init_array();

    initialise_monitor_handles();

    PosixFileSystem fs;

    GenerateSunSData(fs);
    GenerateSailData(fs);
    GenerateCameraCommissioningData(fs);
    GeneratePayloadCommissioningData(fs);
    GenerateLeopData(fs);
    GenerateRadfetData(fs);

    kill(-1, 0);

    return 0;
}
