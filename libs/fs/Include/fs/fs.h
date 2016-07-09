#ifndef LIBS_FS_INCLUDE_FS_FS_H_
#define LIBS_FS_INCLUDE_FS_FS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

/**
 * @defgroup fs File system
 *
 * @brief File system API
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
     * @param[in] openFlag Open flags
     * @param[in] mode Mode
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
 * @param[out] fs File system interface
 * @return true if initialization was successful
 */
bool FileSystemInitialize(FileSystem* fs);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* LIBS_FS_INCLUDE_FS_FS_H_ */
