#include "eps.hpp"

StandaloneEPS::StandaloneEPS(StandaloneI2C& bus, StandaloneI2C& pld) : _bus(bus), _pld(pld)
{
}

struct Command
{
    static constexpr std::uint8_t DisableLCL = 0xE2;
};

static constexpr drivers::i2c::I2CAddress ControllerA = 0b0110101;
static constexpr drivers::i2c::I2CAddress ControllerB = 0b0110110;

bool StandaloneEPS::DisableLCL(LCL lcl)
{
    auto busToUse = (num(lcl) & 0xF0) == 0 ? _bus : _pld;
    auto addressToUse = (num(lcl) & 0xF0) == 0 ? ControllerA : ControllerB;

    auto lclId = static_cast<std::uint8_t>((num(lcl) & 0x0F));

    std::array<std::uint8_t, 2> command{Command::DisableLCL, lclId};

    return busToUse.Write(addressToUse, command) == drivers::i2c::I2CResult::OK;
}