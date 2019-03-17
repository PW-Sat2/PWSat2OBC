#include <algorithm>
#include <array>
#include <cstdint>

#include "base/os.h"
#include "base/reader.h"
#include "base/writer.h"
#include "logger/logger.h"

#include "n25q.h"

using gsl::span;
using std::array;
using std::min;
using std::ptrdiff_t;
using std::uint32_t;
using std::uint8_t;
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

constexpr std::ptrdiff_t N25QDriver::PageSize;
constexpr std::chrono::milliseconds N25QDriver::ProgramPageTimeout;
constexpr std::chrono::milliseconds N25QDriver::EraseSubSectorTimeout;
constexpr std::chrono::seconds N25QDriver::EraseSectorTimeout;
constexpr std::chrono::seconds N25QDriver::EraseChipTimeOut;
constexpr std::chrono::milliseconds N25QDriver::ResetTimeout;
constexpr std::chrono::milliseconds N25QDriver::WriteStatusRegisterTimeout;

extern "C" void assertFailed(const char* source, const char* file, uint16_t line);

static inline void WriterWriteAddress(Writer& writer, size_t address)
{
    writer.WriteByte(static_cast<uint8_t>((address >> 2 * 8) & 0xFF));
    writer.WriteByte(static_cast<uint8_t>((address >> 1 * 8) & 0xFF));
    writer.WriteByte(static_cast<uint8_t>((address >> 0 * 8) & 0xFF));
}

template <typename Action> static OSResult Retry(std::uint8_t retries, Action action)
{
    auto remaining = retries;
    OSResult lastResult = OSResult::InvalidOperation;

    while (remaining > 0)
    {
        lastResult = action();

        if (lastResult == OSResult::Success)
        {
            break;
        }

        remaining--;
        LOG(LOG_LEVEL_WARNING, "[n25q] Retrying action");
    }

    return lastResult;
}

N25QDriver::N25QDriver(                    //
    error_counter::IErrorCounting& errors, //
    error_counter::Device deviceId,        //
    ISPIInterface& spi)
    : _spi(spi),          //
      _errors(errors),    //
      _deviceId(deviceId) //

{
}

Id N25QDriver::ReadId()
{
    auto id = ReadIdWithoutErrorHandling();
    if (!id.IsValid())
    {
        _errors.Failure(_deviceId);
    }
    else
    {
        _errors.Success(_deviceId);
    }

    return id;
}

Id N25QDriver::ReadIdWithoutErrorHandling()
{
    array<uint8_t, 3> response;

    Retry(RetryCount, [this, &response]() {
        SPISelectSlave slave(this->_spi);

        return this->Command(N25QCommand::ReadId, span<uint8_t>(response));
    });

    Reader reader(response);

    auto manufacturer = reader.ReadByte();
    auto memoryType = reader.ReadByte();
    auto memoryCapacity = reader.ReadByte();

    return Id(manufacturer, memoryType, memoryCapacity);
}

Status N25QDriver::ReadStatus()
{
    uint8_t status;

    Retry(RetryCount, [this, &status]() {
        SPISelectSlave slave(this->_spi);

        return this->Command(N25QCommand::ReadStatusRegister, span<uint8_t>(&status, 1));
    });

    return static_cast<Status>(status);
}

FlagStatus N25QDriver::ReadFlagStatus()
{
    uint8_t status;

    Retry(RetryCount, [this, &status]() {
        SPISelectSlave slave(this->_spi);

        return this->Command(N25QCommand::ReadFlagStatusRegister, span<uint8_t>(&status, 1));
    });

    return static_cast<FlagStatus>(status);
}

OSResult N25QDriver::ReadMemory(std::size_t address, gsl::span<uint8_t> buffer)
{
    auto r = Retry(RetryCount, [this, address, &buffer]() {
        array<uint8_t, 4> command;
        Writer writer(command);

        writer.WriteByte(N25QCommand::ReadMemory);
        WriterWriteAddress(writer, address);

        SPISelectSlave slave(this->_spi);
        auto r = this->_spi.Write(command);

        if (r != OSResult::Success)
        {
            return r;
        }

        return this->_spi.Read(buffer);
    });

    if (r == OSResult::Success)
    {
        this->_errors.Success(this->_deviceId);
    }
    else
    {
        this->_errors.Failure(this->_deviceId);
    }
    return r;
}

OperationWaiter::OperationWaiter(IN25QDriver* driver, std::chrono::milliseconds timeout, FlagStatus errorStatus)
    : _waitResult(None<OperationResult>()), //
      _driver(driver),                      //
      _timeout(timeout),                    //
      _errorStatus(errorStatus)             //

{
}

OperationWaiter::OperationWaiter(OperationWaiter&& other)
    : _waitResult(other._waitResult),  //
      _driver(other._driver),          //
      _timeout(other._timeout),        //
      _errorStatus(other._errorStatus) //
{
    other._waitResult = None<OperationResult>();
    other._driver = nullptr;
    other._timeout = std::chrono::milliseconds(0);
    other._errorStatus = FlagStatus::Clear;
}

OperationWaiter::~OperationWaiter()
{
    if (_driver != nullptr && !_waitResult.HasValue)
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

    _waitResult = Some(_driver->WaitForOperation(_timeout, _errorStatus));
    return _waitResult.Value;
}

void OperationWaiter::Cancel()
{
    _waitResult = Some(OperationResult::Success);
}

OperationWaiter& OperationWaiter::operator=(OperationWaiter&& other) noexcept
{
    OperationWaiter tmp(std::move(other));

    using std::swap;

    swap(this->_waitResult, tmp._waitResult);
    swap(this->_driver, tmp._driver);
    swap(this->_timeout, tmp._timeout);
    swap(this->_errorStatus, tmp._errorStatus);

    return *this;
}

OperationWaiter N25QDriver::BeginWritePage(size_t address, ptrdiff_t offset, span<const uint8_t> page)
{
    if (page.length() > PageSize)
    {
        // assertFailed("N25QDriver", __FILE__, __LINE__);
        return OperationWaiter(nullptr, std::chrono::milliseconds(0), FlagStatus::ProgramError);
    }

    this->EnableWrite();

    {
        array<uint8_t, 4> command;
        Writer writer(command);

        writer.WriteByte(N25QCommand::ProgramMemory);
        WriterWriteAddress(writer, address + offset);

        SPISelectSlave slave(this->_spi);

        this->_spi.Write(command);
        this->_spi.Write(page);
    }

    return OperationWaiter(this, ProgramPageTimeout, FlagStatus::ProgramError);
}

OSResult N25QDriver::Command(const std::uint8_t command, gsl::span<std::uint8_t> response)
{
    auto r = this->_spi.Write(span<const uint8_t>(&command, 1));

    if (r != OSResult::Success)
    {
        return r;
    }

    return this->_spi.Read(response);
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

OperationResult N25QDriver::WaitForOperation(std::chrono::milliseconds timeout, FlagStatus errorStatus)
{
    {
        SPISelectSlave slave(_spi);

        if (!WaitBusy(timeout))
        {
            _errors.Failure(_deviceId);
            return OperationResult::Timeout;
        }
    }

    auto flags = ReadFlagStatus();
    if (has_flag(flags, errorStatus))
    {
        _errors.Failure(_deviceId);
        return OperationResult::Failure;
    }

    _errors.Success(_deviceId);
    return OperationResult::Success;
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
        auto id = this->ReadIdWithoutErrorHandling();

        if (id.IsValid())
            break;

        if (timeoutCheck.Expired())
        {
            _errors.Failure(_deviceId);
            return OperationResult::Timeout;
        }

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
            _errors.Failure(_deviceId);
            return OperationResult::Timeout;
        }
    }

    _errors.Success(_deviceId);
    return OperationResult::Success;
}

OSResult N25QDriver::Command(const std::uint8_t command)
{
    return this->_spi.Write(span<const uint8_t>(&command, 1));
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
