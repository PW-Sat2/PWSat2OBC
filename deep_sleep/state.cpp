#include "state.hpp"
#include "boot/settings.hpp"

static constexpr devices::fm25w::Address BaseAddress = 1024;
static constexpr std::int32_t InvalidTopLimit = 1024;

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
    ResetInvalidCounters();
}

void State::ResetInvalidCounters()
{
    for (int i = 0; i < num(CounterType::Max); ++i)
    {
        auto value = ReadCounter(static_cast<CounterType>(i));
        if (value >= InvalidTopLimit)
        {
            WriteCounter(static_cast<CounterType>(i), 0);
        }
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
    if(value <= 0)
    {
        state.WriteCounter(_type, _limit);
        if(_handler != nullptr)
        {
            _handler(_context, *this);
        }
    }
    else
    {
        state.WriteCounter(_type, value);
    }
}
