#pragma once

#include <array>
#include "standalone/i2c/i2c.hpp"

struct EPSTelemetryA
{
    std::array<std::uint8_t, 72> Buffer;
};

struct EPSTelemetryB
{
    std::array<std::uint8_t, 18> Buffer;
};

enum class LCL : std::uint8_t
{
    TKMain = 0x01,      //!< TKMain
    SunS = 0x02,        //!< SunS
    CamNadir = 0x03,    //!< CamNadir
    CamWing = 0x04,     //!< CamWing
    SENS = 0x05,        //!< SENS
    AntennaMain = 0x06, //!< AntennaMain
    IMTQ = 0x07,        //!< IMTQ
    TKRed = 0x11,       //!< TKRed
    AntennaRed = 0x12   //!< AntennaRed
};

enum class EPSController
{
    A,
    B
};

class StandaloneEPS
{
  public:
    StandaloneEPS(StandaloneI2C& bus, StandaloneI2C& pld);

    bool DisableLCL(LCL lcl);

    bool ReadTelemetryA(EPSTelemetryA& telemetry);
    bool ReadTelemetryB(EPSTelemetryB& telemetry);

    void PowerCycle(EPSController controller);

  private:
    StandaloneI2C& _bus;
    StandaloneI2C& _pld;
};