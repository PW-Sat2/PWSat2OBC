#ifndef LIBS_FS_INCLUDE_FS_YAFFS_H_
#define LIBS_FS_INCLUDE_FS_YAFFS_H_

#include "fs.h"

/**
 * @defgroup fs_yaffs YAFFS implementation of file system interface
 * @ingroup fs
 *
 * @brief YAFFS implementation of file system interface
 *
 * @{
 */

/**
 * @brief Yaffs implementation of file system interface
 */
class YaffsFileSystem final : public FileSystem
{
  public:
    /**
     * @brief Initializes file system interface
     */
    void Initialize();

    virtual FSFileOpenResult Open(const char* path, FSFileOpen openFlag, FSFileAccess accessMode) override;
    virtual OSResult TruncateFile(FSFileHandle file, FSFileSize length) override;
    virtual FSIOResult Write(FSFileHandle file, gsl::span<const std::uint8_t> buffer) override;
    virtual FSIOResult Read(FSFileHandle file, gsl::span<std::uint8_t> buffer) override;
    virtual OSResult Close(FSFileHandle file) override;
    virtual FSDirectoryOpenResult OpenDirectory(const char* dirname) override;
    virtual char* ReadDirectory(FSDirectoryHandle directory) override;
    virtual OSResult CloseDirectory(FSDirectoryHandle directory) override;
    virtual OSResult Format(const char* mountPoint) override;
    virtual OSResult MakeDirectory(const char* path) override;
    virtual bool Exists(const char* path) override;

    /**
     * @brief Removes all files and directories from specified device
     * @param[in] device Device to clear
     * @return Operation result
     */
    OSResult ClearDevice(yaffs_dev* device);

    /**
     * @brief Syncs file system (speeds up next mount)
     */
    void Sync();

    /**
     * @brief Adds device and mounts it
     * @param[in] device YAFFS device
     * @return true on success
     */
    bool AddDeviceAndMount(yaffs_dev* device);
};

/** @} */

#endif /* LIBS_FS_INCLUDE_FS_YAFFS_H_ */
