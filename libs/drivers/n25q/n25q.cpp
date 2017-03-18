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
using redundancy::Voter;

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

constexpr std::chrono::milliseconds N25QDriver::ProgramPageTimeout;
constexpr std::chrono::milliseconds N25QDriver::EraseSubSectorTimeout;
constexpr std::chrono::seconds N25QDriver::EraseSectorTimeout;
constexpr std::chrono::seconds N25QDriver::EraseChipTimeOut;
constexpr std::chrono::milliseconds N25QDriver::ResetTimeout;
constexpr std::chrono::milliseconds N25QDriver::WriteStatusRegisterTimeout;

static inline void WriterWriteAddress(Writer& writer, size_t address)
{
    writer.WriteByte(static_cast<uint8_t>((address >> 2 * 8) & 0xFF));
    writer.WriteByte(static_cast<uint8_t>((address >> 1 * 8) & 0xFF));
    writer.WriteByte(static_cast<uint8_t>((address >> 0 * 8) & 0xFF));
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

void N25QDriver::ReadMemory(std::size_t address, gsl::span<uint8_t> buffer)
{
    array<uint8_t, 4> command;
    Writer writer(command);

    writer.WriteByte(N25QCommand::ReadMemory);
    WriterWriteAddress(writer, address);

    SPISelectSlave slave(this->_spi);
    this->_spi.Write(command);
    this->_spi.Read(buffer);
}

OperationWaiter::OperationWaiter(N25QDriver* driver, std::chrono::milliseconds timeout, FlagStatus errorStatus)
    : _waitResult(None<OperationResult>()), //
      _driver(driver),                      //
      _timeout(timeout),                    //
      _errorStatus(errorStatus)             //

{
}

OperationWaiter::~OperationWaiter()
{
    if (!_waitResult.HasValue)
    {
        Wait();
    }
}

OperationResult OperationWaiter::Wait()
{
    if (_waitResult.HasValue)
    {
        return _waitResult.Value;
    }

    {
        SPISelectSlave slave(_driver->_spi);

        if (!_driver->WaitBusy(_timeout))
        {
            _waitResult = Some(OperationResult::Timeout);
            return _waitResult.Value;
        }
    }

    auto flags = _driver->ReadFlagStatus();

    if (has_flag(flags, _errorStatus))
    {
        _waitResult = Some(OperationResult::Failure);
        return _waitResult.Value;
    }

    _waitResult = Some(OperationResult::Success);
    return _waitResult.Value;
}

OperationWaiter N25QDriver::BeginWriteChunk(size_t address, ptrdiff_t offset, span<const uint8_t> chunk)
{
    this->EnableWrite();

    {
        array<uint8_t, 4> command;
        Writer writer(command);

        writer.WriteByte(N25QCommand::ProgramMemory);
        WriterWriteAddress(writer, address + offset);

        SPISelectSlave slave(this->_spi);

        this->_spi.Write(command);
        this->_spi.Write(chunk);
    }

    return OperationWaiter(this, ProgramPageTimeout, FlagStatus::ProgramError);
}

OperationResult N25QDriver::WriteMemory(size_t address, span<const uint8_t> buffer)
{
    this->ClearFlags();

    for (ptrdiff_t offset = 0; offset < buffer.size(); offset += 256)
    {
        auto chunk = buffer.subspan(offset, min(256, buffer.size() - offset));

        auto chunkWriteResult = BeginWriteChunk(address, offset, chunk).Wait();

        if (chunkWriteResult != OperationResult::Success)
            return chunkWriteResult;
    }

    return OperationResult::Success;
}

void N25QDriver::Command(const std::uint8_t command, gsl::span<std::uint8_t> response)
{
    this->_spi.Write(span<const uint8_t>(&command, 1));
    this->_spi.Read(response);
}

void N25QDriver::EnableWrite()
{
    SPISelectSlave slave(this->_spi);

    this->Command(N25QCommand::WriteEnable);
}

bool N25QDriver::WaitBusy(std::chrono::milliseconds timeout)
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

OperationWaiter N25QDriver::BeginEraseSector(size_t address)
{
    this->ClearFlags();

    this->EnableWrite();

    {
        SPISelectSlave select(this->_spi);

        this->Command(N25QCommand::EraseSector);
        this->WriteAddress(address);
    }

    return OperationWaiter(this, EraseSectorTimeout, FlagStatus::EraseError);
}

OperationWaiter N25QDriver::BeginEraseSubSector(size_t address)
{
    this->ClearFlags();

    this->EnableWrite();

    {
        SPISelectSlave select(this->_spi);

        this->Command(N25QCommand::EraseSubsector);
        this->WriteAddress(address);
    }

    return OperationWaiter(this, EraseSubSectorTimeout, FlagStatus::EraseError);
}

OperationWaiter N25QDriver::BeginEraseChip()
{
    this->ClearFlags();

    this->EnableWrite();

    {
        SPISelectSlave select(this->_spi);
        this->Command(N25QCommand::EraseChip);
    }

    return OperationWaiter(this, EraseChipTimeOut, FlagStatus::EraseError);
}

void N25QDriver::ClearFlags()
{
    SPISelectSlave select(this->_spi);

    this->Command(N25QCommand::ClearFlagRegister);
}

OperationResult N25QDriver::Reset()
{
    {
        SPISelectSlave select(this->_spi);

        this->Command(N25QCommand::ResetEnable);
    }

    {
        SPISelectSlave select(this->_spi);

        this->Command(N25QCommand::ResetMemory);
    }

    Timeout timeoutCheck(ResetTimeout);

    do
    {
        auto id = this->ReadId();

        if (id.IsValid())
            break;

        if (timeoutCheck.Expired())
            return OperationResult::Timeout;

        System::Yield();
    } while (true);

    this->EnableWrite();

    {
        SPISelectSlave select(this->_spi);

        this->Command(N25QCommand::WriteStatusRegister);

        auto value = static_cast<uint8_t>(Status::ProtectedAreaFromBottom);
        this->_spi.Write(gsl::make_span(&value, 1));
    }

    {
        SPISelectSlave select(this->_spi);

        if (!this->WaitBusy(WriteStatusRegisterTimeout))
        {
            return OperationResult::Timeout;
        }
    }

    return OperationResult::Success;
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
