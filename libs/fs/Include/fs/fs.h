#ifndef LIBS_FS_INCLUDE_FS_FS_H_
#define LIBS_FS_INCLUDE_FS_FS_H_

#include <stdbool.h>
#include <stdint.h>
#include "system.h"

EXTERNC_BEGIN

#include <yaffs_guts.h>

/**
 * @defgroup fs File system
 *
 * @brief File system API. POSIX-compatible
 *
 * Wrappers for YAFFS file system.
 * Partitions:
 * - / - NAND memory, 1MB, 1 page per chunk
 *
 * @{
 */

/** @brief Directory handle */
typedef void* FSDirectoryHandle;

/** @brief File handle */
typedef int FSFileHandle;

/**
 * @brief Structure exposing file system API
 */
typedef struct
{
    /**
     * @brief Opens file
     * @param[in] path Path to file
     * @param[in] openFlag Open flags (O_CREAT, O_RDONLY, O_WRONLY, O_RDWR, O_APPEND)
     * @param[in] mode Mode (Use S_IRWXU)
     * @return File handle. In case of error: -1
     */
    FSFileHandle (*open)(const char* path, int openFlag, int mode);

    /**
     * @brief Truncates file to given size
     * @param[in] file File handle
     * @param[in] length Desired length
     * @return 0 on success, -1 on error
     */
    int (*ftruncate)(FSFileHandle file, int64_t length);

    /**
     * @brief Writes data to file
     * @param[in] file File handle
     * @param[in] buffer Data buffer
     * @param[in] size Size of data
     * @return Number of written bytes on success. -1 on error
     */
    int (*write)(FSFileHandle file, const void* buffer, unsigned int size);

    /**
     * @brief Reads data from file
     * @param[in] file File handle
     * @param[out] buffer Data buffer
     * @param[in] size Size of data
     * @return Number of read bytes on success. -1 on error
     */
    int (*read)(FSFileHandle file, void* buffer, unsigned int size);

    /**
     * @brief Closes file
     * @param[in] file File handle
     * @return 0 on success, -1 on error
     */
    int (*close)(FSFileHandle file);

    /**
     * @brief Opens directory
     * @param[in] dirname Directory path
     * @return Directory handle on success. -1 on error
     */
    FSDirectoryHandle (*openDirectory)(const char* dirname);

    /**
     * @brief Reads name of next entry in directory
     * @param[in] directory Directory handle
     * @return Entry name. NULL if no more entries found
     */
    char* (*readDirectory)(FSDirectoryHandle directory);

    /**
     * @brief Closes directory
     * @param[in] directory Directory handle
     * @return 0 on success, -1 on error
     */
    int (*closeDirectory)(FSDirectoryHandle directory);

    /**
     * @brief Gets last error code
     * @return Error code, @see errno.h
     */
    int (*getLastError)(void);
} FileSystem;

/**
 * @brief Initializes file system interface
 * @param[inout] fs File system interface
 * @param[in] rootDevice root device driver
 * @return true if initialization was successful
 */
bool FileSystemInitialize(FileSystem* fs, struct yaffs_dev* rootDevice);

/**
 * @brief This method is responsible for writing serialized state to file.
 *
 * If the selected file exists it will be overwritten, if it does not exist it will be created.
 * @param[in] fs FileSystem interface for accessing files.
 * @param[in] file Path to file that should be saved.
 * @param[in] buffer Pointer to buffer that contains timer state that should be saved.
 * @param[in] length Size of data in bytes the buffer.
 */
bool FileSystemSaveToFile(FileSystem* fs, const char* file, const uint8_t* buffer, uint32_t length);

/**
 * @brief This procedure is responsible for the contents of the specified file.
 *
 * @param[in] fs FileSystem interface for accessing files.
 * @param[in] filePath Path to the file that contains timer state.
 *
 * @return Read timer state or default (zero) state in case of errors.
 */
bool FileSystemReadFile(FileSystem* fs, const char* const filePath, uint8_t* buffer, uint32_t length);

/** @} */

EXTERNC_END

#endif /* LIBS_FS_INCLUDE_FS_FS_H_ */
