#ifndef LIBS_OBC_STORAGE_N25Q_FLASH_INCLUDE_OBC_STORAGE_N25Q_H_
#define LIBS_OBC_STORAGE_N25Q_FLASH_INCLUDE_OBC_STORAGE_N25Q_H_

#include "fs/yaffs.h"
#include "n25q/n25q.h"
#include "n25q/yaffs.h"

namespace obc
{
    namespace storage
    {
        /**
        * @defgroup obc_storage_n25q OBC Storage (N25Q)
        * @ingroup obc_storage
        *
        * @{
        */

        /**
         * @brief Storage driver for N25Q
         */
        class N25QStorage final
        {
          public:
            /**
             * @brief Constructs @ref N25QStorage instance
             * @param[in] spi SPI interface used by external memories
             * @param[in] fs File system interface
             */
            N25QStorage(drivers::spi::ISPIInterface& spi, services::fs::YaffsFileSystem& fs);

            /** @brief Initializes OBC storage */
            void Initialize();

            /**
             * @brief Clears OBC storage
             * @return Operation result
             */
            OSResult ClearStorage();

            /** @brief Performs (lengthy) erase operation */
            OSResult Erase();

            /** @brief N25Q flash driver */
            devices::n25q::N25QDriver ExternalFlashDriver;

            /** @brief N25Q Yaffs device */
            devices::n25q::N25QYaffsDevice<devices::n25q::BlockMapping::Sector, 512_Bytes, 16_MB> ExternalFlash;

          private:
            services::fs::YaffsFileSystem& _fs;
        };

        /** @} */
    }
}

#endif /* LIBS_OBC_STORAGE_N25Q_FLASH_INCLUDE_OBC_STORAGE_N25Q_H_ */
