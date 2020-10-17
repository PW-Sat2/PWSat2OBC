#include "state.hpp"
#include "boot/settings.hpp"
#include "mcu/io_map.h"

static constexpr devices::fm25w::Address BaseAddress = 1024;
static constexpr std::int32_t InvalidTopLimit = 1024;
static constexpr std::int32_t OneTimeInitMarker = 0xdeadbeaf;

void SendToUart(USART_TypeDef* uart, const char* message);

error_counter::CounterValue NullErrorCounter::Current(error_counter::Device /* device*/) const
{
    return 0;
}

void NullErrorCounter::Failure(error_counter::Device /*device*/)
{
}

void NullErrorCounter::Success(error_counter::Device /*device*/)
{
}

State::State(SPIPeripheral& spi)
    : _fram1Spi(spi),                                   //
      _fram2Spi(spi),                                   //
      _fram3Spi(spi),                                   //
      _fram1(_fram1Spi),                                //
      _fram2(_fram2Spi),                                //
      _fram3(_fram3Spi),                                //
      _fram(_errorCounter, {&_fram1, &_fram2, &_fram3}) //
{
}

void State::Initialize()
{
    this->_fram1Spi.Initialize();
    this->_fram2Spi.Initialize();
    this->_fram3Spi.Initialize();
    if(ReadCounter(CounterType::Init) != OneTimeInitMarker)
    {
        SendToUart(io_map::UART_1::Peripheral, "Resetting deep sleep counters\n");
        ResetCounters();
        WriteCounter(CounterType::Init, OneTimeInitMarker);
    }
}

void State::ResetCounters()
{
    for (int i = 0; i < num(CounterType::Max); ++i)
    {
        WriteCounter(static_cast<CounterType>(i), 0);
    }
}

std::int32_t State::ReadCounter(CounterType counter)
{
    union {
        std::int32_t value;
        std::uint8_t buffer[sizeof(std::int32_t)];
    } local;

    this->_fram.Read(BaseAddress + num(counter) * sizeof(std::int32_t), gsl::make_span(local.buffer));
    return local.value;
}

void State::WriteCounter(CounterType counter, std::int32_t value)
{
    union {
        std::int32_t value;
        std::uint8_t buffer[sizeof(std::int32_t)];
    } local;

    local.value = value;
    this->_fram.Write(BaseAddress + num(counter) * sizeof(std::int32_t), gsl::make_span(local.buffer));
}

void State::ConfirmBoot()
{
    boot::BootSettings settings{this->_fram};
    settings.ConfirmBoot();
}

std::uint32_t State::BootCounter()
{
    boot::BootSettings settings{this->_fram};
    return settings.BootCounter();
}

void State::SwapBootSlots()
{
    boot::BootSettings settings{this->_fram};

    auto primary = settings.BootSlots();
    auto backup = settings.FailsafeBootSlots();

    settings.BootSlots(backup);
    settings.FailsafeBootSlots(primary);
}

void Counter::Verify(State& state)
{
    const auto value = state.ReadCounter(_type) - 1;
    if (value <= 0)
    {
        Reset(state);
        if (_handler != nullptr)
        {
            _handler(_context, *this);
        }
    }
    else
    {
        state.WriteCounter(_type, value);
    }
}

void Counter::Reset(State& state)
{
    state.WriteCounter(_type, _limit);
}