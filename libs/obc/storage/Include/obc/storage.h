#ifndef SRC_STORAGE_H_
#define SRC_STORAGE_H_

#include "base/os.h"
#include "fs/fs.h"
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
         * @param spi SPI interface to use
         * @param fs File system interface
         */
        OBCStorageHandler(drivers::spi::EFMSPIInterface& spi, FileSystem& fs);

        /** @brief Performs storage initialization */
        void Initialize();

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

      private:
        Storage _storage;
    };

    template <typename Storage>
    OBCStorageHandler<Storage>::OBCStorageHandler(drivers::spi::EFMSPIInterface& spi, FileSystem& fs) : _storage(spi, fs)
    {
    }

    template <typename Storage> inline void obc::OBCStorageHandler<Storage>::Initialize()
    {
        this->_storage.Initialize();
    }

    template <typename Storage> inline OSResult obc::OBCStorageHandler<Storage>::ClearStorage()
    {
        return this->_storage.ClearStorage();
    }

    template <typename Storage> inline OSResult obc::OBCStorageHandler<Storage>::Erase()
    {
        return this->_storage.Erase();
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
