#include <array>
#include <cstdint>
#include <em_cmu.h>
#include <em_gpio.h>
#include <em_int.h>
#include <em_timer.h>
#include <gsl/span>
#include "obc.h"
#include "scrubber/ram.hpp"
#include "utils.h"

using std::uint8_t;
using std::uint32_t;

void ScrubRAM(std::uint16_t /*argc*/, char* /*argv*/ [])
{
    auto current = Scrubber::Current();

    Main.terminal.Printf("Current = 0x%08X\n", current);
}
