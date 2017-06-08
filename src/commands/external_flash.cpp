#include "dmadrv.h"
#include "obc.h"
#include "obc/storage/n25q.h"

using drivers::spi::EFMSPISlaveInterface;
using devices::n25q::N25QDriver;
using namespace obc::storage::error_counters;

void TestExternalFlash(std::uint16_t argc, char* argv[])
{
    UNUSED(argc, argv);

    EFMSPISlaveInterface spi1(Main.Hardware.SPI, Main.Hardware.Pins.Flash1ChipSelect);
    EFMSPISlaveInterface spi2(Main.Hardware.SPI, Main.Hardware.Pins.Flash2ChipSelect);
    EFMSPISlaveInterface spi3(Main.Hardware.SPI, Main.Hardware.Pins.Flash3ChipSelect);

    N25QDriver flash1(Main.Fdir.ErrorCounting(), N25QDriver1::ErrorCounter::DeviceId, spi1);
    N25QDriver flash2(Main.Fdir.ErrorCounting(), N25QDriver2::ErrorCounter::DeviceId, spi2);
    N25QDriver flash3(Main.Fdir.ErrorCounting(), N25QDriver3::ErrorCounter::DeviceId, spi3);

    auto id1 = flash1.ReadId();
    auto id2 = flash2.ReadId();
    auto id3 = flash3.ReadId();

    Main.terminal.Printf("Flash 1 id valid: %d\r\n", id1.IsValid());
    Main.terminal.Printf("Flash 2 id valid: %d\r\n", id2.IsValid());
    Main.terminal.Printf("Flash 3 id valid: %d\r\n", id3.IsValid());

    if (!id1.IsValid() || !id2.IsValid() || !id3.IsValid())
    {
        Main.terminal.Printf("SOME FLASHES ARE INVALID!");
    }
}
