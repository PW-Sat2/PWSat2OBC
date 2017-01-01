#ifndef LIBS_FS_INCLUDE_FS_YAFFS_H_
#define LIBS_FS_INCLUDE_FS_YAFFS_H_

#include "fs.h"

class YaffsFileSystem final : public FileSystem
{
  public:
    /**
     * @brief Initializes file system interface
     */
    void Initialize();

    virtual FSFileOpenResult open(const char* path, FSFileOpenFlags openFlag, FSFileAccessMode accessMode) override;
    virtual OSResult ftruncate(FSFileHandle file, FSFileSize length) override;
    virtual FSIOResult write(FSFileHandle file, const void* buffer, FSFileSize size) override;
    virtual FSIOResult read(FSFileHandle file, void* buffer, FSFileSize size) override;
    virtual OSResult close(FSFileHandle file) override;
    virtual FSDirectoryOpenResult openDirectory(const char* dirname) override;
    virtual char* readDirectory(FSDirectoryHandle directory) override;
    virtual OSResult closeDirectory(FSDirectoryHandle directory) override;
    virtual OSResult format(const char* mountPoint) override;
    virtual OSResult makeDirectory(const char* path) override;
    virtual bool exists(const char* path) override;
    virtual OSResult ClearDevice(yaffs_dev* device) override;
    virtual void Sync() override;
};

#endif /* LIBS_FS_INCLUDE_FS_YAFFS_H_ */
