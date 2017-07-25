#ifndef SRC_STORAGE_H_
#define SRC_STORAGE_H_

#include "base/os.h"

#include "fs/fs.h"
#include "fs/yaffs.h"
#include "obc/hardware.h"
#include "spi/efm.h"
#include "utils.h"

#ifdef USE_EXTERNAL_FLASH
#include "obc/storage/n25q.h"
#else
#include "obc/storage/stk.h"
#endif

namespace obc
{
    /**
     * @defgroup obc_storage OBC storage
     * @ingroup obc
     *
     * Hardware devices and file system drivers
     *
     * @{
     */

    /**
     * @brief OBC storage
     * @tparam Storage Specific storage handler (@ref obc::storage::N25QStorage or @ref obc::storage::STKStorage)
     */
    template <typename Storage> class OBCStorageHandler final
    {
      public:
        /**
         * @brief Initializes @ref OBCStorageHandler instance
         * @param errors Error counting service
         * @param spi SPI interface to use
         * @param deviceOperations YAFFS device operations
         * @param pins GPIO pins
         */
        OBCStorageHandler(                                          //
            error_counter::ErrorCounting& errors,                   //
            drivers::spi::EFMSPIInterface& spi,                     //
            services::fs::IYaffsDeviceOperations& deviceOperations, //
            obc::OBCGPIO& pins                                      //
            );

        /**
         * @brief Performs storage initialization
         * @return Operation result
         */
        OSResult Initialize();

        /**
         * @brief Clears OBC storage
         * @return Operation result
         */
        OSResult ClearStorage();

        /**
         * @brief Performs (potentially lengthy) erase operation
         * @return Operation result
         */
        OSResult Erase();

        /**
         * @brief Gives access to the actual storage object
         * @return Internal storage object.
         */
        Storage& GetInternalStorage();

      private:
        /** @brief Underlying storage implementation */
        Storage _storage;
    };

    template <typename Storage>
    OBCStorageHandler<Storage>::OBCStorageHandler(              //
        error_counter::ErrorCounting& errors,                   //
        drivers::spi::EFMSPIInterface& spi,                     //
        services::fs::IYaffsDeviceOperations& deviceOperations, //
        obc::OBCGPIO& pins                                      //
        )
        : _storage(errors, spi, deviceOperations, pins)
    {
    }

    template <typename Storage> inline OSResult obc::OBCStorageHandler<Storage>::Initialize()
    {
        return this->_storage.Initialize();
    }

    template <typename Storage> inline OSResult obc::OBCStorageHandler<Storage>::ClearStorage()
    {
        return this->_storage.ClearStorage();
    }

    template <typename Storage> inline OSResult obc::OBCStorageHandler<Storage>::Erase()
    {
        return this->_storage.Erase();
    }

    template <typename Storage> inline Storage& obc::OBCStorageHandler<Storage>::GetInternalStorage()
    {
        return this->_storage;
    }

#ifdef USE_EXTERNAL_FLASH
    /** @brief Specific implementation of OBC storage */
    using OBCStorage = OBCStorageHandler<obc::storage::N25QStorage>;
#else
    /** @brief Specific implementation of OBC storage */
    using OBCStorage = OBCStorageHandler<obc::storage::STKStorage>;
#endif
}

/** @} */

#endif /* SRC_STORAGE_H_ */
