#include <chrono>
#include <cstdint>
#include <em_adc.h>
#include <em_cmu.h>
#include <em_prs.h>
#include "obc.h"

using namespace std::chrono_literals;

void Temp(std::uint16_t /*argc*/, char* /*argv*/ [])
{
    auto temp = Main.Hardware.Temperature.ReadCelsius();

    Main.terminal.Printf("Temp=%d\n", temp);
}
