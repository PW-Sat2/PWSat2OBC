#include <array>
#include <cstdint>

#include <base/reader.h>
#include "n25q.h"

using std::uint8_t;
using std::array;
using gsl::span;
using namespace devices::n25q;
using drivers::spi::ISPIInterface;
using drivers::spi::SPISelectSlave;

enum N25QCommand
{
    ReadId = 0x9E,
    ReadStatusRegister = 0x05,
    ReadFlagStatusRegister = 0x70,
    ReadMemory = 0x03,
    WriteEnable = 0x06,
    WriteDisable = 0x05,
    ProgramMemory = 0x02,
    EraseSubsector = 0x20,
    EraseChip = 0xC7
};

N25QDriver::N25QDriver(ISPIInterface& spi) : _spi(spi)
{
}

Id N25QDriver::ReadId()
{
    array<uint8_t, 3> response;

    {
        SPISelectSlave slave(this->_spi);

        this->Command(N25QCommand::ReadId, span<uint8_t>(response));
    }

    Reader reader;
    ReaderInitialize(&reader, response.data(), 3);

    Id id;

    id.Manufacturer = ReaderReadByte(&reader);
    id.MemoryType = ReaderReadByte(&reader);
    id.MemoryCapacity = ReaderReadByte(&reader);

    return id;
}

Status N25QDriver::ReadStatus()
{
    uint8_t status;

    {
        SPISelectSlave slave(this->_spi);

        this->Command(N25QCommand::ReadStatusRegister, span<uint8_t>(&status, 1));
    }

    return static_cast<Status>(status);
}

FlagStatus N25QDriver::ReadFlagStatus()
{
    uint8_t status;

    {
        SPISelectSlave slave(this->_spi);

        this->Command(N25QCommand::ReadFlagStatusRegister, span<uint8_t>(&status, 1));
    }

    return static_cast<FlagStatus>(status);
}

void N25QDriver::Command(const std::uint8_t command, gsl::span<std::uint8_t> response)
{
    this->_spi.WriteRead(span<const uint8_t>(&command, 1), response);
}
