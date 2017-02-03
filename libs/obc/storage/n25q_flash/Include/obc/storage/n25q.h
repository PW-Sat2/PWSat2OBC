#ifndef LIBS_OBC_STORAGE_N25Q_FLASH_INCLUDE_OBC_STORAGE_N25Q_H_
#define LIBS_OBC_STORAGE_N25Q_FLASH_INCLUDE_OBC_STORAGE_N25Q_H_

#include "fs/yaffs.h"
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
         * @brief Represents single flash mememory mounted as Yaffs device
         */
        class SingleFlash final
        {
          public:
            /**
             * @brief Ctor
             * @param mountPoint Mount point
             * @param slaveSelect Pin used as slave select
             * @param spi SPI interface
             * @param deviceOperations Device operations
             */
            SingleFlash(const char* mountPoint,
                const drivers::gpio::Pin& slaveSelect,
                drivers::spi::EFMSPIInterface& spi,
                services::fs::IYaffsDeviceOperations& deviceOperations);

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

            /**
             * @brief Performs (lengthy) erase operation
             * @return Operation result
             */
            OSResult Erase();

          private:
            /** @brief SPI driver for N25Q flash slave */
            drivers::spi::EFMSPISlaveInterface SPI;

            /** @brief N25Q flash driver */
            devices::n25q::N25QDriver Driver;

            /** @brief N25Q Yaffs device */
            devices::n25q::N25QYaffsDevice<devices::n25q::BlockMapping::Sector, 512_Bytes, 16_MB> Device;

            /** @brief Device operations */
            services::fs::IYaffsDeviceOperations& _deviceOperations;
        };

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
             * @param deviceOperations YAFFS device operations
             * @param[in] pins GPIO pins (unused)
             */
            N25QStorage(drivers::spi::EFMSPIInterface& spi, services::fs::IYaffsDeviceOperations& deviceOperations, obc::OBCGPIO& pins);

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

            /**
             * @brief Performs (lengthy) erase operation
             * @return Operation result
             */
            OSResult Erase();

          private:
            /** @brief External flashes */
            SingleFlash _flashes[3];
        };

        /** @} */
    }
}

#endif /* LIBS_OBC_STORAGE_N25Q_FLASH_INCLUDE_OBC_STORAGE_N25Q_H_ */
