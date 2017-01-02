#include "fs.h"

using namespace services::fs;

FileOpenResult::FileOpenResult(OSResult status, FileHandle handle) : Status(status), Handle(handle)
{
}

DirectoryOpenResult::DirectoryOpenResult(OSResult status, DirectoryHandle handle) : Status(status), Handle(handle)
{
}

IOResult::IOResult(OSResult status, FileSize bytesTransferred) : Status(status), BytesTransferred(bytesTransferred)
{
}
