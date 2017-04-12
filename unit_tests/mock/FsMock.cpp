#include "FsMock.hpp"
#include <utility>

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

FsMock::FsMock()
{
}

FsMock::~FsMock()
{
}
