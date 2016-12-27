#ifndef LIBS_OBC_STORAGE_N25Q_FLASH_INCLUDE_OBC_STORAGE_N25Q_H_
#define LIBS_OBC_STORAGE_N25Q_FLASH_INCLUDE_OBC_STORAGE_N25Q_H_

#include "fs/fs.h"
#include "fs/yaffs.h"
#include "spi/spi.h"
#include "spi/efm.h"
#include "storage/nand.h"
#include "storage/nand_driver.h"

namespace obc
{
    namespace storage
    {
        /**
         * @defgroup obc_storage_stk OBC Storage (STK)
         * @ingroup obc_storage
         *
         * @{
         */

        /**
         * @brief Storage driver for STK (NAND)
         */
        class STKStorage final
        {
          public:
            /**
             * @brief Constructs @ref STKStorage instance
             * @param[in] spi SPI interface used by external memories
             * @param deviceOperations YAFFS device operations
             */
            STKStorage(drivers::spi::EFMSPIInterface& spi, services::fs::IYaffsDeviceOperations& deviceOperations);

            /**
             * @brief Initializes OBC storage
             * @return Operation result
             */
            OSResult Initialize();

            /**
             * @brief Clears OBC storage
             * @return Operation result
             */
            OSResult ClearStorage();

            /** @brief Performs erase operation */
            OSResult Erase();

            /** Yaffs root device */
            struct yaffs_dev rootDevice;
            /** Driver for yaffs root device */
            YaffsNANDDriver rootDeviceDriver;

          private:
            /** @brief File system interface */
            services::fs::IYaffsDeviceOperations& _deviceOperations;
        };

        /** @} */
    }
}

#endif /* LIBS_OBC_STORAGE_N25Q_FLASH_INCLUDE_OBC_STORAGE_N25Q_H_ */
