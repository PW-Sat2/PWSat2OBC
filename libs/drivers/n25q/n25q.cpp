#include <algorithm>
#include <array>
#include <cstdint>

#include "base/os.h"
#include "base/reader.h"
#include "base/writer.h"

#include "n25q.h"

using std::uint8_t;
using std::uint32_t;
using std::array;
using std::min;
using std::ptrdiff_t;
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
    EraseSector = 0xD8,
    EraseChip = 0xC7,
    ClearFlagRegister = 0x50,
    ResetEnable = 0x66,
    ResetMemory = 0x99,
    WriteStatusRegister = 0x01
};

static inline void WriterWriteAddress(Writer* writer, size_t address)
{
    WriterWriteByte(writer, static_cast<uint8_t>((address >> 2 * 8) & 0xFF));
    WriterWriteByte(writer, static_cast<uint8_t>((address >> 1 * 8) & 0xFF));
    WriterWriteByte(writer, static_cast<uint8_t>((address >> 0 * 8) & 0xFF));
}

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

    Reader reader(response);

    auto manufacturer = reader.ReadByte();
    auto memoryType = reader.ReadByte();
    auto memoryCapacity = reader.ReadByte();

    return Id(manufacturer, memoryType, memoryCapacity);
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

void N25QDriver::ReadMemory(size_t address, span<uint8_t> buffer)
{
    array<uint8_t, 4> command;
    Writer writer;

    WriterInitialize(&writer, command.data(), command.size());
    WriterWriteByte(&writer, N25QCommand::ReadMemory);
    WriterWriteAddress(&writer, address);

    SPISelectSlave slave(this->_spi);
    this->_spi.Write(command);
    this->_spi.Read(buffer);
}

OperationResult N25QDriver::WriteMemory(size_t address, span<const uint8_t> buffer)
{
    this->ClearFlags();

    for (ptrdiff_t offset = 0; offset < buffer.size(); offset += 256)
    {
        auto chunk = buffer.subspan(offset, min(256, buffer.size() - offset));

        this->EnableWrite();

        {
            array<uint8_t, 4> command;
            Writer writer;
            WriterInitialize(&writer, command.data(), command.size());
            WriterWriteByte(&writer, N25QCommand::ProgramMemory);
            WriterWriteAddress(&writer, address + offset);

            SPISelectSlave slave(this->_spi);

            this->_spi.Write(command);
            this->_spi.Write(chunk);
        }

        {
            SPISelectSlave slave(this->_spi);

            if (!this->WaitBusy(ProgramPageTimeout))
            {
                return OperationResult::Timeout;
            }
        }

        auto flags = this->ReadFlagStatus();

        if (has_flag(flags, FlagStatus::ProgramError))
        {
            return OperationResult::Failure;
        }
    }

    return OperationResult::Success;
}

void N25QDriver::Command(const std::uint8_t command, gsl::span<std::uint8_t> response)
{
    this->_spi.WriteRead(span<const uint8_t>(&command, 1), response);
}

void N25QDriver::EnableWrite()
{
    SPISelectSlave slave(this->_spi);

    this->Command(N25QCommand::WriteEnable);
}

bool N25QDriver::WaitBusy(OSTaskTimeSpan timeout)
{
    Timeout timeoutCheck(timeout);

    do
    {
        uint8_t status = 0;

        this->Command(N25QCommand::ReadStatusRegister, span<uint8_t>(&status, 1));

        if (!has_flag(static_cast<Status>(status), Status::WriteInProgress))
        {
            break;
        }

        if (timeoutCheck.Expired())
        {
            return false;
        }

        System::Yield();
    } while (true);

    return true;
}

OperationResult N25QDriver::EraseSector(size_t address)
{
    this->ClearFlags();

    this->EnableWrite();

    {
        SPISelectSlave select(this->_spi);

        this->Command(N25QCommand::EraseSector);
        this->WriteAddress(address);
    }

    {
        SPISelectSlave select(this->_spi);

        if (!this->WaitBusy(EraseSectorTimeout))
        {
            return OperationResult::Timeout;
        }
    }

    auto flags = this->ReadFlagStatus();

    if (has_flag(flags, FlagStatus::EraseError))
    {
        return OperationResult::Failure;
    }

    return OperationResult::Success;
}

OperationResult N25QDriver::EraseSubSector(size_t address)
{
    this->ClearFlags();

    this->EnableWrite();

    {
        SPISelectSlave select(this->_spi);

        this->Command(N25QCommand::EraseSubsector);
        this->WriteAddress(address);
    }

    {
        SPISelectSlave select(this->_spi);

        if (!this->WaitBusy(EraseSubSectorTimeout))
        {
            return OperationResult::Timeout;
        }
    }

    auto flags = this->ReadFlagStatus();

    if (has_flag(flags, FlagStatus::EraseError))
    {
        return OperationResult::Failure;
    }

    return OperationResult::Success;
}

OperationResult N25QDriver::EraseChip()
{
    this->ClearFlags();

    this->EnableWrite();

    {
        SPISelectSlave select(this->_spi);
        this->Command(N25QCommand::EraseChip);
    }

    {
        SPISelectSlave select(this->_spi);

        if (!this->WaitBusy(EraseChipTimeOut))
        {
            return OperationResult::Timeout;
        }
    }

    auto flags = this->ReadFlagStatus();

    if (has_flag(flags, FlagStatus::EraseError))
    {
        return OperationResult::Failure;
    }

    return OperationResult::Success;
}

void N25QDriver::ClearFlags()
{
    SPISelectSlave select(this->_spi);

    this->Command(N25QCommand::ClearFlagRegister);
}

void N25QDriver::Reset()
{
    {
        SPISelectSlave select(this->_spi);

        this->Command(N25QCommand::ResetEnable);
    }

    {
        SPISelectSlave select(this->_spi);

        this->Command(N25QCommand::ResetMemory);
    }

    while (!this->ReadId().IsValid())
        ;

    this->EnableWrite();

    {
        SPISelectSlave select(this->_spi);

        this->Command(N25QCommand::WriteStatusRegister);

        auto value = static_cast<uint8_t>(Status::ProtectedAreaFromBottom);
        this->_spi.Write(gsl::make_span(&value, 1));
    }

    {
        SPISelectSlave select(this->_spi);

        if (!this->WaitBusy(100))
        {
            return;
        }
    }
}

void N25QDriver::Command(const std::uint8_t command)
{
    this->_spi.Write(span<const uint8_t>(&command, 1));
}

void N25QDriver::WriteAddress(const std::size_t address)
{
    uint8_t bytes[] = {
        static_cast<uint8_t>((address >> 2 * 8) & 0xFF), //
        static_cast<uint8_t>((address >> 1 * 8) & 0xFF), //
        static_cast<uint8_t>((address >> 0 * 8) & 0xFF),
    };

    this->_spi.Write(bytes);
}

Id::Id(std::uint8_t manufacturer, std::uint8_t memoryType, std::uint8_t memoryCapacity)
    : Manufacturer(manufacturer), MemoryType(memoryType), MemoryCapacity(memoryCapacity)
{
}
