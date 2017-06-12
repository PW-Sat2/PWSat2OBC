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

    struct BootConfirmedField : public Element<std::uint8_t>
    {
    };

    using BootFields = Offsets<MagicNumberField, BootSlotsField, FailsafeBootSlotsField, BootCounterField, BootConfirmedField>;

    BootSettings::BootSettings(devices::fm25w::IFM25WDriver& fram) : _fram(fram)
    {
    }

    bool BootSettings::CheckMagicNumber() const
    {
        alignas(4) array<std::uint8_t, 4> buf;
        this->_fram.Read(BootFields::Offset<MagicNumberField>(), buf);

        return Reader(buf).ReadDoubleWordLE() == MagicNumber;
    }

    std::uint8_t BootSettings::BootSlots() const
    {
        if (!this->CheckMagicNumber())
        {
            return DefaultBootSlot;
        }

        alignas(4) std::uint8_t r = 0;

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

        alignas(4) std::uint8_t r = 0;

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

        alignas(4) array<std::uint8_t, 4> buf;
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

    bool BootSettings::WasLastBootConfirmed() const
    {
        std::uint8_t buf = 0;
        this->_fram.Read(BootFields::Offset<BootConfirmedField>(), {&buf, 1});

        return buf == BootConfirmedFlag;
    }

    bool BootSettings::ConfirmLastBoot()
    {
        std::uint8_t buf = BootConfirmedFlag;
        this->_fram.Write(BootFields::Offset<BootConfirmedField>(), {&buf, 1});

        return WasLastBootConfirmed();
    }

    bool BootSettings::UnconfirmLastBoot()
    {
        std::uint8_t buf = 0;
        this->_fram.Write(BootFields::Offset<BootConfirmedField>(), {&buf, 1});

        return !WasLastBootConfirmed();
    }

    void BootSettings::Erase()
    {
        array<std::uint8_t, BootFields::Size> buf;
        buf.fill(0xFF);

        this->_fram.Write(0, buf);
    }
}
