#include "settings.hpp"
#include <array>
#include <type_traits>
#include "base/offset.hpp"
#include "base/reader.h"
#include "base/writer.h"
#include "fm25w/fm25w.hpp"

using std::array;

namespace boot
{
    struct MagicNumberField : public Element<std::uint32_t>
    {
    };

    struct BootSlotsField : public Element<std::uint8_t>
    {
    };

    struct FailsafeBootSlotsField : public Element<std::uint8_t>
    {
    };

    struct BootCounterField : public Element<std::uint32_t>
    {
    };

    struct LastConfirmedBootCounterField : public Element<std::uint32_t>
    {
    };

    using BootFields = Offsets<MagicNumberField, BootSlotsField, FailsafeBootSlotsField, BootCounterField, LastConfirmedBootCounterField>;

    BootSettings::BootSettings(devices::fm25w::IFM25WDriver& fram) : _fram(fram)
    {
    }

    bool BootSettings::CheckMagicNumber() const
    {
        array<std::uint8_t, 4> buf;
        this->_fram.Read(BootFields::Offset<MagicNumberField>(), buf);

        return Reader(buf).ReadDoubleWordLE() == MagicNumber;
    }

    std::uint8_t BootSettings::BootSlots() const
    {
        if (!this->CheckMagicNumber())
        {
            return DefaultBootSlot;
        }

        std::uint8_t r = 0;

        this->_fram.Read(BootFields::Offset<BootSlotsField>(), {&r, 1});

        return r;
    }

    bool BootSettings::BootSlots(std::uint8_t slots)
    {
        this->_fram.Write(BootFields::Offset<BootSlotsField>(), {&slots, 1});
        return this->BootSlots() == slots;
    }

    std::uint8_t BootSettings::FailsafeBootSlots() const
    {
        if (!this->CheckMagicNumber())
        {
            return DefaultFailsafeBootSlot;
        }

        std::uint8_t r = 0;

        this->_fram.Read(BootFields::Offset<FailsafeBootSlotsField>(), {&r, 1});

        return r;
    }

    bool BootSettings::FailsafeBootSlots(std::uint8_t slots)
    {
        this->_fram.Write(BootFields::Offset<FailsafeBootSlotsField>(), {&slots, 1});
        return this->FailsafeBootSlots() == slots;
    }

    std::uint32_t BootSettings::BootCounter() const
    {
        if (!this->CheckMagicNumber())
        {
            return DefaultBootCounter;
        }

        array<std::uint8_t, 4> buf;
        this->_fram.Read(BootFields::Offset<BootCounterField>(), buf);

        return Reader(buf).ReadDoubleWordLE();
    }

    bool BootSettings::BootCounter(std::uint32_t counter)
    {
        array<std::uint8_t, 4> buf;

        Writer(buf).WriteDoubleWordLE(counter);

        this->_fram.Write(BootFields::Offset<BootCounterField>(), buf);

        return this->BootCounter() == counter;
    }

    void BootSettings::MarkAsValid()
    {
        array<std::uint8_t, 4> buf;

        Writer(buf).WriteDoubleWordLE(MagicNumber);
        this->_fram.Write(BootFields::Offset<MagicNumberField>(), buf);
    }

    std::uint32_t BootSettings::LastConfirmedBootCounter() const
    {
        array<std::uint8_t, 4> buf;
        this->_fram.Read(BootFields::Offset<LastConfirmedBootCounterField>(), buf);

        Reader r(buf);

        return r.ReadDoubleWordLE();
    }

    bool BootSettings::ConfirmBoot()
    {
        auto counter = this->BootCounter();

        array<std::uint8_t, 4> buf;

        Writer w(buf);
        w.WriteDoubleWordLE(counter);

        this->_fram.Write(BootFields::Offset<LastConfirmedBootCounterField>(), buf);

        return this->LastConfirmedBootCounter() == counter;
    }

    void BootSettings::Erase()
    {
        array<std::uint8_t, BootFields::Size> buf;
        buf.fill(0xFF);

        this->_fram.Write(0, buf);
    }
}
