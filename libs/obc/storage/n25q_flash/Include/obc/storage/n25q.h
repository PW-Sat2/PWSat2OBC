#ifndef LIBS_OBC_STORAGE_N25Q_FLASH_INCLUDE_OBC_STORAGE_N25Q_H_
#define LIBS_OBC_STORAGE_N25Q_FLASH_INCLUDE_OBC_STORAGE_N25Q_H_

#include "fs/fs.h"
#include "n25q/n25q.h"
#include "n25q/yaffs.h"
#include "obc/gpio.h"
#include "spi/efm.h"

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
         * @brief Storage handler for N25Q
         *
         * This class manages external flash and it's integration with YAFFS
         */
        class N25QStorage final
        {
          public:
            /**
             * @brief Constructs @ref N25QStorage instance
             * @param[in] spi SPI interface used by external memories
             * @param[in] fs File system interface
             * @param[in] pins GPIO pins (unused)
             */
            N25QStorage(drivers::spi::EFMSPIInterface& spi, FileSystem& fs, obc::OBCGPIO& pins);

            /** @brief Initializes OBC storage */
            void Initialize();

            /**
             * @brief Clears OBC storage
             * @return Operation result
             */
            OSResult ClearStorage();

            /** @brief Performs (lengthy) erase operation */
            OSResult Erase();

          private:
            /** @brief SPI driver for N25Q flash slave */
            drivers::spi::EFMSPISlaveInterface ExternalFlashDriverSPI;

            /** @brief N25Q flash driver */
            devices::n25q::N25QDriver ExternalFlashDriver;

            /** @brief N25Q Yaffs device */
            devices::n25q::N25QYaffsDevice<devices::n25q::BlockMapping::Sector, 512_Bytes, 16_MB> ExternalFlash;

            /** @brief File system interface */
            FileSystem& _fs;
        };

        /** @} */
    }
}

#endif /* LIBS_OBC_STORAGE_N25Q_FLASH_INCLUDE_OBC_STORAGE_N25Q_H_ */
