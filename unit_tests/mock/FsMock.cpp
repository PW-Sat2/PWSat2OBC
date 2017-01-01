#include "FsMock.hpp"
#include <utility>

FSFileOpenResult MakeOpenedFile(int handle)
{
    FSFileOpenResult result;
    result.Status = OSResult::Success;
    result.Handle = handle;
    return result;
}

FSFileOpenResult MakeOpenedFile(OSResult status)
{
    FSFileOpenResult result;
    result.Status = status;
    result.Handle = -1;
    return result;
}

FSIOResult MakeFSIOResult(OSResult status)
{
    FSIOResult result;
    result.Status = status;
    result.BytesTransferred = 0;
    return result;
}

FSIOResult MakeFSIOResult(int bytesTransfered)
{
    FSIOResult result;
    result.Status = OSResult::Success;
    result.BytesTransferred = bytesTransfered;
    return result;
}
