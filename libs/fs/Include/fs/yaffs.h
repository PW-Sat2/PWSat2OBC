#ifndef LIBS_FS_INCLUDE_FS_YAFFS_H_
#define LIBS_FS_INCLUDE_FS_YAFFS_H_

#include "fs.h"

namespace services
{
    namespace fs
    {
        /**
         * @defgroup fs_yaffs YAFFS implementation of file system interface
         * @ingroup fs
         *
         * @brief YAFFS implementation of file system interface
         *
         * @{
         */

        /**
         * @brief API for mounting YAFFS device
         */
        struct IYaffsDeviceOperations
        {
            /**
             * @brief Adds device and mounts it
             * @param[in] device YAFFS device
             * @return Operation result
             */
            virtual OSResult AddDeviceAndMount(yaffs_dev* device) = 0;

            /**
             * @brief Removes all files and directories from specified device
             * @param[in] device Device to clear
             * @return Operation result
             */
            virtual OSResult ClearDevice(yaffs_dev* device) = 0;
        };

        /**
         * @brief Yaffs implementation of file system interface
         */
        class YaffsFileSystem final : public IFileSystem, public IYaffsDeviceOperations
        {
          public:
            /**
             * @brief Initializes file system interface
             */
            void Initialize();

            virtual FileOpenResult Open(const char* path, FileOpen openFlag, FileAccess accessMode) override;
            virtual OSResult TruncateFile(FileHandle file, FileSize length) override;
            virtual IOResult Write(FileHandle file, gsl::span<const std::uint8_t> buffer) override;
            virtual IOResult Read(FileHandle file, gsl::span<std::uint8_t> buffer) override;
            virtual OSResult Close(FileHandle file) override;
            virtual DirectoryOpenResult OpenDirectory(const char* dirname) override;
            virtual char* ReadDirectory(DirectoryHandle directory) override;
            virtual OSResult CloseDirectory(DirectoryHandle directory) override;
            virtual OSResult Format(const char* mountPoint) override;
            virtual OSResult MakeDirectory(const char* path) override;
            virtual bool Exists(const char* path) override;

            virtual OSResult ClearDevice(yaffs_dev* device) override;

            /**
             * @brief Syncs file system (speeds up next mount)
             */
            void Sync();

            virtual OSResult AddDeviceAndMount(yaffs_dev* device) override;
        };
    }
}

/** @} */

#endif /* LIBS_FS_INCLUDE_FS_YAFFS_H_ */
