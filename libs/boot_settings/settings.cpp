#include "settings.hpp"
#include <array>
#include "base/reader.h"
#include "base/writer.h"
#include "fm25w/fm25w.hpp"

using std::array;

namespace boot
{
    BootSettings::BootSettings(devices::fm25w::IFM25WDriver& fram) : _fram(fram)
    {
    }

    bool BootSettings::CheckMagicNumber() const
    {
        array<std::uint8_t, 4> buf;
        this->_fram.Read(0, buf);

        return Reader(buf).ReadDoubleWordLE() == MagicNumber;
    }

    std::uint8_t BootSettings::BootSlots() const
    {
        if (!this->CheckMagicNumber())
        {
            return DefaultBootSlot;
        }

        std::uint8_t r = 0;

        this->_fram.Read(5, {&r, 1});

        return r;
    }

    bool BootSettings::BootSlots(std::uint8_t slots)
    {
        this->_fram.Write(5, {&slots, 1});
        return this->BootSlots() == slots;
    }

    std::uint8_t BootSettings::FailsafeBootSlots() const
    {
        if (!this->CheckMagicNumber())
        {
            return DefaultFailsafeBootSlot;
        }

        std::uint8_t r = 0;

        this->_fram.Read(6, {&r, 1});

        return r;
    }

    bool BootSettings::FailsafeBootSlots(std::uint8_t slots)
    {
        this->_fram.Write(6, {&slots, 1});
        return this->FailsafeBootSlots() == slots;
    }

    std::uint16_t BootSettings::BootCounter() const
    {
        if (!this->CheckMagicNumber())
        {
            return DefaultBootCounter;
        }

        array<std::uint8_t, 2> buf;
        this->_fram.Read(7, buf);

        return Reader(buf).ReadWordLE();
    }

    bool BootSettings::BootCounter(std::uint16_t counter)
    {
        array<std::uint8_t, 2> buf;

        Writer(buf).WriteWordLE(counter);

        this->_fram.Write(7, buf);

        return this->BootCounter() == counter;
    }

    void BootSettings::MarkAsValid()
    {
        array<std::uint8_t, 4> buf;

        Writer(buf).WriteDoubleWordLE(MagicNumber);
        this->_fram.Write(0, buf);
    }

    bool BootSettings::WasLastBootConfirmed() const
    {
        std::uint8_t buf = 0;
        this->_fram.Read(9, {&buf, 1});

        return buf == BootConfirmedFlag;
    }

    bool BootSettings::ConfirmLastBoot()
    {
        std::uint8_t buf = BootConfirmedFlag;
        this->_fram.Write(9, {&buf, 1});

        return WasLastBootConfirmed();
    }

    bool BootSettings::UnconfirmLastBoot()
    {
        std::uint8_t buf = 0;
        this->_fram.Write(9, {&buf, 1});

        return !WasLastBootConfirmed();
    }
}
