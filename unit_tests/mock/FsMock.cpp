#include "FsMock.hpp"
#include <utility>

using namespace services::fs;

FileOpenResult MakeOpenedFile(int handle)
{
    FileOpenResult result;
    result.Status = OSResult::Success;
    result.Handle = handle;
    return result;
}

FileOpenResult MakeOpenedFile(OSResult status)
{
    FileOpenResult result;
    result.Status = status;
    result.Handle = -1;
    return result;
}

IOResult MakeFSIOResult(OSResult status)
{
    IOResult result;
    result.Status = status;
    result.BytesTransferred = 0;
    return result;
}

IOResult MakeFSIOResult(int bytesTransfered)
{
    IOResult result;
    result.Status = OSResult::Success;
    result.BytesTransferred = bytesTransfered;
    return result;
}
