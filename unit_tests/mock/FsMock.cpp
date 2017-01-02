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

IOResult MakeFSIOResult(int bytesTransfered)
{
    return IOResult(OSResult::Success, bytesTransfered);
}
