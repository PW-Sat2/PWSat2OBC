#include "FsMock.hpp"
#include <utility>

using testing::Invoke;
using testing::_;

using namespace services::fs;

FileOpenResult MakeOpenedFile(int handle)
{
    return FileOpenResult(OSResult::Success, handle);
}

FileOpenResult MakeOpenedFile(OSResult status)
{
    return FileOpenResult(status, -1);
}

IOResult MakeFSIOResult(OSResult status)
{
    return IOResult(status, 0);
}

IOResult MakeFSIOResult(gsl::span<const uint8_t> result)
{
    return IOResult(OSResult::Success, result);
}

void FsMock::AddFile(const char* path, gsl::span<std::uint8_t> contents)
{
    this->_files[path] = contents;
}

FsMock::FsMock() : _nextHandle(100)
{
    ON_CALL(*this, Open(_, _, _)).WillByDefault(Invoke([this](const char* path, FileOpen /*mode*/, FileAccess /*access*/) {
        auto f = this->_files.find(path);

        if (f == this->_files.end())
        {
            return MakeOpenedFile(OSResult::NotFound);
        }

        auto handle = this->_nextHandle;
        this->_nextHandle++;

        this->_opened[handle].Position = f->second.begin();
        this->_opened[handle].File = path;

        return MakeOpenedFile(handle);
    }));

    ON_CALL(*this, Close(_)).WillByDefault(Invoke([this](FileHandle handle) {
        this->_opened.erase(handle);
        return OSResult::Success;
    }));

    ON_CALL(*this, Read(_, _)).WillByDefault(Invoke([this](FileHandle handle, gsl::span<std::uint8_t> buffer) {
        auto f = this->_opened.find(handle);

        if (f == this->_opened.end())
        {
            return MakeFSIOResult(OSResult::InvalidFileHandle);
        }

        gsl::span<std::uint8_t>::iterator end = std::copy(f->second.Position, f->second.Position + buffer.size(), buffer.begin());

        f->second.Position += (end - buffer.begin());

        return MakeFSIOResult(gsl::span<std::uint8_t>(buffer.data(), (end - buffer.begin())));
    }));

    ON_CALL(*this, Seek(_, _, _)).WillByDefault(Invoke([this](FileHandle handle, SeekOrigin origin, FileSize offset) {
        auto f = this->_opened.find(handle);

        if (f == this->_opened.end())
        {
            return OSResult::InvalidFileHandle;
        }

        auto& content = this->_files[f->second.File];

        decltype(f->second.Position) newPosition;
        switch (origin)
        {
            case SeekOrigin::Begin:
                newPosition = content.begin() + offset;
                break;
            case SeekOrigin::Current:
                newPosition = f->second.Position + offset;
                break;
            case SeekOrigin::End:
                newPosition = content.end() + offset;
                break;
            default:
                return OSResult::OutOfRange;
        }

        if (newPosition < content.begin() || newPosition >= content.end())
        {
            return OSResult::OutOfRange;
        }

        f->second.Position = newPosition;

        return OSResult::Success;
    }));

    ON_CALL(*this, GetFileSize(_)).WillByDefault(Invoke([this](FileHandle handle) {
        auto f = this->_opened.find(handle);

        if (f == this->_opened.end())
        {
            return -1;
        }

        auto content = this->_files[f->second.File];

        return content.size();
    }));
}

FsMock::~FsMock()
{
}
