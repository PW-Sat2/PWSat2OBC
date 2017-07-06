#include "base/BitWriter.hpp"
#include "telemetry/FileSystem.hpp"

namespace telemetry
{
    FileSystemTelemetry::FileSystemTelemetry(std::uint32_t size) : freeSpace(size)
    {
    }

    void FileSystemTelemetry::Write(BitWriter& writer) const
    {
        writer.Write(this->freeSpace);
    }
}
