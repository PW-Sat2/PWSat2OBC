#ifndef LIBS_DRIVERS_PROGRAM_FLASH_INCLUDE_PROGRAM_FLASH_FLASH_DRIVER_HPP_
#define LIBS_DRIVERS_PROGRAM_FLASH_INCLUDE_PROGRAM_FLASH_FLASH_DRIVER_HPP_

#include <cstdint>
#include <gsl/span>
#include "base/os.h"
#include "utils.h"

namespace program_flash
{
    /**
     * @defgroup program_flash_driver Driver for program flash (S29JL)
     * @ingroup device_drivers
     *
     * @{
     */

    /**
     * @brief Flash status
     */
    enum class FlashStatus : std::uint8_t
    {
        StatusUnknown = 0,                                 //!< StatusUnknown
        NotBusy,                                           //!< NotBusy
        Busy,                                              //!< Busy
        ExceededTimeLimits,                                //!< ExceededTimeLimits
        Suspend,                                           //!< Suspend
        WriteBufferAbort,                                  //!< WriteBufferAbort
        StatusGetProblem,                                  //!< StatusGetProblem
        VerifyError,                                       //!< VerifyError
        BytesPerOpWrong,                                   //!< BytesPerOpWrong
        EraseError,                                        //!< EraseError
        ProgramError,                                      //!< ProgramError
        SectorLock,                                        //!< SectorLock
        ProgramSuspend,                                    //!< ProgramSuspend
        /* Device is in program suspend mode */            //!< ProgramSuspend
        ProgramSuspendError,                               //!< ProgramSuspendError
        /* Device program suspend error */                 //!< ProgramSuspendError
        EraseSuspend,                                      //!< EraseSuspend
        /* Device is in erase suspend mode */              //!< EraseSuspend
        EraseSuspendError,                                 //!< EraseSuspendError
        /* Device erase suspend error */                   //!< EraseSuspendError
        BusyInOtherBank /* Busy operation in other bank */ //!< BusyInOtherBank
    };

    /** @brief Flash driver */
    struct IFlashDriver;

    /**
     * @brief Class providing easy access to area of flash at defined offset
     */
    class FlashSpan
    {
      public:
        /**
         * @brief Ctor
         * @param flash Flash driver
         * @param offset Offset at which accessed area starts
         */
        FlashSpan(IFlashDriver& flash, std::size_t offset);

        /**
         * @brief Returns pointer to data at offset from span start
         * @param offset Offset into area
         * @return Pointer to data
         */
        inline std::uint8_t const* At(std::size_t offset) const;

        /**
         * @brief Returns pointer to the start of span
         * @return Pointer to data
         */
        inline std::uint8_t const* Data() const;

        /**
         * @brief Erases single sector (64KB) inside span
         * @param sectorOffset Sector offset
         * @return Operation result
         *
         * @remark Caller must ensure that offsets are valid (aligned to 64KB, inside flash range)
         */
        inline FlashStatus Erase(std::size_t sectorOffset);

        /**
         * @brief Programs single byte into flash
         * @param offset Offset from span start
         * @param value Value to write
         * @return Operation result
         */
        inline FlashStatus Program(std::size_t offset, std::uint8_t value);

        /**
         * @brief Programs multiple bytes into flash
         * @param offset Offset from span start
         * @param value Value to write
         * @return Operation result
         */
        inline FlashStatus Program(std::size_t offset, gsl::span<const std::uint8_t> value);

        /**
         * @brief Returns reference to used flash driver
         * @return Flash driver
         */
        inline IFlashDriver& Driver();

        /**
         * @brief Returns base offset of span
         * @return Base offset
         */
        inline std::size_t BaseOffset() const;

      private:
        /** @brief Reference to flash driver */
        IFlashDriver& _flash;
        /** @brief Base offset */
        const std::size_t _offset;
    };

    /**
     * @brief Helper class allowing specifing flash span offset in declarative way
     */
    template <std::size_t Offset> class FlashSpanAt : public FlashSpan
    {
      public:
        /**
         * @brief Ctor
         * @param baseSpan Span on which this span should be based
         */
        FlashSpanAt(FlashSpan& baseSpan) : FlashSpan(baseSpan.Driver(), baseSpan.BaseOffset() + Offset)
        {
        }
    };

    /**
     * @brief Flash driver interface
     */
    struct IFlashDriver
    {
        /**
         * @brief Returns pointer to data at given offset
         * @param offset Offset into flash (0 means first byte of flash)
         * @return Pointer to data
         */
        virtual std::uint8_t const* At(std::size_t offset) const = 0;

        /**
         * @brief Programs single byte into flash
         * @param offset Offset
         * @param value Value to write
         * @return Operation result
         */
        virtual FlashStatus Program(std::size_t offset, std::uint8_t value) = 0;

        /**
         * @brief Programs multiple bytes into flash
         * @param offset Offset
         * @param value Value to write
         * @return Operation result
         */
        virtual FlashStatus Program(std::size_t offset, gsl::span<const std::uint8_t> value) = 0;

        /**
         * @brief Erases single sector (64KB) inside span
         * @param sectorOffset Sector offset
         * @return Operation result
         *
         * @remark Caller must ensure that offsets are valid (aligned to 64KB, inside flash range)
         */
        virtual FlashStatus EraseSector(std::size_t sectorOffset) = 0;

        /**
         * @brief Returns device ID
         * @return Device ID
         */
        virtual std::uint32_t DeviceId() const = 0;

        /**
         * @brief Returns device boot config
         * @return Boot config
         */
        virtual std::uint32_t BootConfig() const = 0;

        /**
         * @brief Creates span starting at given offset
         * @param offset Offset from first byt of flash
         * @return
         */
        inline FlashSpan Span(std::size_t offset)
        {
            return FlashSpan(*this, offset);
        }
    };

    /**
     * @brief Flash driver for S29JL
     */
    class FlashDriver : public IFlashDriver
    {
      public:
        /**
         * @brief Ctor
         * @param flashBase Pointer for memory area mapped to flash device (EBI)
         */
        explicit FlashDriver(std::uint8_t* flashBase);

        /**
         * @brief Performs driver initialization
         */
        void Initialize();

        virtual inline std::uint32_t DeviceId() const override
        {
            return this->_deviceId;
        }

        virtual inline std::uint32_t BootConfig() const override
        {
            return this->_bootConfig;
        }

        virtual inline std::uint8_t const* At(std::size_t offset) const override
        {
            return this->_flashBase + offset;
        }

        virtual FlashStatus EraseSector(std::size_t sectorOfffset) override;

        virtual FlashStatus Program(std::size_t offset, std::uint8_t value) override;
        virtual FlashStatus Program(std::size_t offset, gsl::span<const std::uint8_t> value) override;

        /** @brief Size of sector (erase unit) */
        static constexpr std::size_t LargeSectorSize = 64_KB;

      private:
        /**
         * @brief Waits for device to become ready
         * @param offset Offset used to select bank which needs to become ready
         * @return true if device is idle
         */
        bool WaitForIdle(std::size_t offset);

        /** @brief Synchronization */
        OSSemaphoreHandle _sync;
        /** @brief Pointer for memory area mapped to flash device */
        std::uint8_t* _flashBase;
        /** @brief Device ID */
        std::uint32_t _deviceId;
        /** @brief Boot config */
        std::uint8_t _bootConfig;
    };

    inline uint8_t const* FlashSpan::At(std::size_t offset) const
    {
        return this->_flash.At(this->_offset + offset);
    }

    inline FlashStatus FlashSpan::Erase(std::size_t sectorOffset)
    {
        return this->_flash.EraseSector(this->_offset + sectorOffset);
    }

    inline std::uint8_t const* FlashSpan::Data() const
    {
        return this->At(0);
    }

    inline FlashStatus FlashSpan::Program(std::size_t offset, std::uint8_t value)
    {
        return this->_flash.Program(this->_offset + offset, value);
    }

    inline IFlashDriver& FlashSpan::Driver()
    {
        return this->_flash;
    }

    inline FlashStatus FlashSpan::Program(std::size_t offset, gsl::span<const std::uint8_t> value)
    {
        return this->_flash.Program(this->_offset + offset, value);
    }

    inline std::size_t FlashSpan::BaseOffset() const
    {
        return this->_offset;
    }
    /* @} */
}

#endif /* LIBS_DRIVERS_PROGRAM_FLASH_INCLUDE_PROGRAM_FLASH_FLASH_DRIVER_HPP_ */
