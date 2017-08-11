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
         * @brief Storage handler for N25Q
         *
         * This class manages external flash and it's integration with YAFFS
         */
        class N25QStorage final
        {
          public:
            /**
             * @brief Constructs @ref N25QStorage instance
             * @param[in] errors Error counting service
             * @param[in] spi SPI interface used by external memories
             * @param deviceOperations YAFFS device operations
             * @param[in] pins GPIO pins (unused)
             */
            N25QStorage(                                                //
                error_counter::ErrorCounting& errors,                   //
                drivers::spi::EFMSPIInterface& spi,                     //
                services::fs::IYaffsDeviceOperations& deviceOperations, //
                obc::OBCGPIO& pins                                      //
                );

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

            /**
             * @brief Gives access to low-level N25Q drivers.
             * @param[in] index Index (0, 1 or 2) of the driver to return.
             * @return Low-level N25Q driver
             */
            devices::n25q::N25QDriver& GetDriver(uint8_t index);

            /**
             * @brief Returns top (redundant) driver
             * @return Reference to driver
             */
            inline devices::n25q::RedundantN25QDriver& GetTopDriver();

          private:
            services::fs::IYaffsDeviceOperations& _deviceOperations;

            drivers::spi::EFMSPISlaveInterface _spiSlaves[3];

            devices::n25q::N25QDriver _n25qDrivers[3];

            devices::n25q::RedundantN25QDriver _driver;

            devices::n25q::N25QYaffsDevice<devices::n25q::BlockMapping::Sector, 2_KB, 16_MB> Device;
        };

        namespace error_counters
        {
            /** @brief Defines error counter with embedded device id in first flash chip driver */
            struct N25QDriver1
            {
                /** @brief Error counter type */
                using ErrorCounter = error_counter::ErrorCounter<4>;
            };

            /** @brief Defines error counter with embedded device id in second flash chip driver */
            struct N25QDriver2
            {
                /** @brief Error counter type */
                using ErrorCounter = error_counter::ErrorCounter<5>;
            };

            /** @brief Defines error counter with embedded device id in third flash chip driver */
            struct N25QDriver3
            {
                /** @brief Error counter type */
                using ErrorCounter = error_counter::ErrorCounter<6>;
            };
        }
        devices::n25q::RedundantN25QDriver& N25QStorage::GetTopDriver()
        {
            return this->_driver;
        }

        /** @} */
    }
}

#endif /* LIBS_OBC_STORAGE_N25Q_FLASH_INCLUDE_OBC_STORAGE_N25Q_H_ */
