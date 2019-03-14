#include <cstdint>
#include "error_counter/error_counter.hpp"
#include "fm25w/fm25w.hpp"
#include "mcu/io_map.h"
#include "payload/io_map.h"
#include "standalone/spi/spi.hpp"
#include "system.h"

struct NullErrorCounter : public error_counter::IErrorCounting
{
    virtual error_counter::CounterValue Current(error_counter::Device /* device*/) const override;

    virtual void Failure(error_counter::Device /*device*/) override;

    virtual void Success(error_counter::Device /*device*/) override;
};

enum class CounterType
{
    None = 0,
    PrintCounter1 = 1,
    PrintCounter2 = 2,
    Max,
};

class State
{
  public:
    State(SPIPeripheral& spi);

    void Initialize();

    std::int32_t ReadCounter(CounterType counter);

    void WriteCounter(CounterType counter, std::int32_t value);

    void ConfirmBoot();
    std::uint32_t BootCounter();

  private:
    void ResetInvalidCounters();

    NullErrorCounter _errorCounter;
    SPIDevice<io_map::SlaveSelectFram1> _fram1Spi;
    SPIDevice<io_map::SlaveSelectFram2> _fram2Spi;
    SPIDevice<io_map::SlaveSelectFram3> _fram3Spi;
    devices::fm25w::FM25WDriver _fram1;
    devices::fm25w::FM25WDriver _fram2;
    devices::fm25w::FM25WDriver _fram3;

    devices::fm25w::RedundantFM25WDriver _fram;
};

class Counter
{
  public:
    using HandlerType = void (*)(void* context, const Counter& counter);

    constexpr Counter() = default;

    constexpr Counter(CounterType type, std::int32_t limit, HandlerType handler, void* context);

    void Verify(State& state);

    constexpr CounterType Type() const;

    constexpr std::int32_t Limit() const;

  private:
    CounterType _type = CounterType::None;
    std::int32_t _limit = 0;
    HandlerType _handler = nullptr;
    void* _context = nullptr;
};

constexpr Counter::Counter(CounterType type, std::int32_t limit, HandlerType handler, void* context)
    : _type(type), _limit(limit), _handler(handler), _context(context)
{
}

constexpr CounterType Counter::Type() const
{
    return _type;
}

constexpr std::int32_t Counter::Limit() const
{
    return _limit;
}
