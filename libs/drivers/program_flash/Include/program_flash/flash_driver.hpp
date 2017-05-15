#ifndef LIBS_DRIVERS_PROGRAM_FLASH_INCLUDE_PROGRAM_FLASH_FLASH_DRIVER_HPP_
#define LIBS_DRIVERS_PROGRAM_FLASH_INCLUDE_PROGRAM_FLASH_FLASH_DRIVER_HPP_

#include <cstdint>
#include <gsl/span>
#include "utils.h"

namespace program_flash
{
    enum class FlashStatus : std::uint8_t
    {
        StatusUnknown = 0,
        NotBusy,
        Busy,
        ExceededTimeLimits,
        Suspend,
        WriteBufferAbort,
        StatusGetProblem,
        VerifyError,
        BytesPerOpWrong,
        EraseError,
        ProgramError,
        SectorLock,
        ProgramSuspend,      /* Device is in program suspend mode */
        ProgramSuspendError, /* Device program suspend error */
        EraseSuspend,        /* Device is in erase suspend mode */
        EraseSuspendError,   /* Device erase suspend error */
        BusyInOtherBank      /* Busy operation in other bank */
    };

    struct IFlashDriver;

    class FlashSpan
    {
      public:
        FlashSpan(IFlashDriver& flash, std::size_t offset);

        inline std::uint8_t const* At(std::size_t offset) const;

        inline std::uint8_t const* Data() const;

        inline FlashStatus Erase(std::size_t sectorOffset);

        inline FlashStatus Program(std::size_t offset, std::uint8_t value);
        inline FlashStatus Program(std::size_t offset, gsl::span<const std::uint8_t> value);

        inline IFlashDriver& Driver();
        inline std::size_t BaseOffset() const;

      private:
        IFlashDriver& _flash;
        const std::size_t _offset;
    };

    template <std::size_t Offset> class FlashSpanAt : public FlashSpan
    {
      public:
        FlashSpanAt(FlashSpan& baseSpan) : FlashSpan(baseSpan.Driver(), baseSpan.BaseOffset() + Offset)
        {
        }
    };

    struct IFlashDriver
    {
        virtual std::uint8_t const* At(std::size_t offset) const = 0;
        virtual FlashStatus Program(std::size_t offset, std::uint8_t value) = 0;
        virtual FlashStatus Program(std::size_t offset, gsl::span<const std::uint8_t> value) = 0;
        virtual FlashStatus EraseSector(std::size_t sectorOfffset) = 0;

        virtual std::uint32_t DeviceId() const = 0;
        virtual std::uint32_t BootConfig() const = 0;

        inline FlashSpan Span(std::size_t offset)
        {
            return FlashSpan(*this, offset);
        }
    };

    class FlashDriver : public IFlashDriver
    {
      public:
        explicit FlashDriver(std::uint8_t* flashBase);

        void Initialize();

        inline std::uint8_t* Base() const
        {
            return this->_flashBase;
        }

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

        static constexpr std::size_t LargeSectorSize = 64_KB;

      private:
        std::uint8_t* _flashBase;

        std::uint32_t _deviceId;
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
}

#endif /* LIBS_DRIVERS_PROGRAM_FLASH_INCLUDE_PROGRAM_FLASH_FLASH_DRIVER_HPP_ */
