#ifndef LIBS_FS_INCLUDE_FS_FS_H_
#define LIBS_FS_INCLUDE_FS_FS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

typedef void* FSDirectoryHandle;
typedef int FSFileHandle;

typedef struct
{
    FSFileHandle (*open)(const char* path, int openFlag, int mode);
    int (*ftruncate)(FSFileHandle file, int64_t length);
    int (*write)(FSFileHandle file, const void* buffer, unsigned int size);
    int (*close)(FSFileHandle file);
    int (*read)(FSFileHandle file, void* buffer, unsigned int size);
    FSDirectoryHandle (*openDirectory)(const char* dirname);
    char* (*readDirectory)(FSDirectoryHandle directory);
    int (*closeDirectory)(FSDirectoryHandle directory);
} FileSystem;

bool FileSystemInitialize(FileSystem* fs);

#ifdef __cplusplus
}
#endif

#endif /* LIBS_FS_INCLUDE_FS_FS_H_ */
