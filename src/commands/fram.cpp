#include <array>
#include "base/writer.h"
#include "commands.h"
#include "fm25w/fm25w.hpp"
#include "obc.h"

using std::uint8_t;
using devices::fm25w::FM25WDriver;
using devices::fm25w::Status;
using drivers::spi::EFMSPISlaveInterface;

static void Status()
{
    EFMSPISlaveInterface spi(Main.Hardware.SPI, Main.Hardware.Pins.Flash1ChipSelect);

    FM25WDriver fram(spi);

    auto sr = fram.ReadStatus();

    Main.terminal.Printf("Status register=%X\n", num(sr));
}

static void Write(std::uint16_t address, gsl::span<const std::uint8_t> value)
{
    EFMSPISlaveInterface spi(Main.Hardware.SPI, Main.Hardware.Pins.Flash1ChipSelect);

    FM25WDriver fram(spi);

    fram.Write(address, value);

    Status();
}

static void Read(std::uint16_t address, gsl::span<uint8_t> value)
{
    EFMSPISlaveInterface spi(Main.Hardware.SPI, Main.Hardware.Pins.Flash1ChipSelect);

    FM25WDriver fram(spi);
    fram.Read(address, value);
}

void FRAM(std::uint16_t argc, char* argv[])
{
    if (argc == 0)
    {
        Main.terminal.Puts("fram <status|write|read>\n");
        return;
    }

    if (strcmp(argv[0], "status") == 0)
    {
        Status();
    }

    if (strcmp(argv[0], "read") == 0)
    {
        std::uint16_t address = atoi(argv[1]);
        std::array<std::uint8_t, 16> buf;

        auto toRead = gsl::make_span(buf).subspan(0, atoi(argv[2]));

        Read(address, toRead);

        for (auto b : toRead)
        {
            Main.terminal.Printf("%X ", b);
        }
    }

    if (strcmp(argv[0], "write") == 0)
    {
        std::uint16_t address = atoi(argv[1]);
        std::array<std::uint8_t, 16> buf;

        for (auto i = 2; i < argc; i++)
        {
            buf[i - 2] = atoi(argv[i]);
        }

        auto toWrite = gsl::make_span(buf).subspan(0, argc - 2);

        for (auto b : toWrite)
        {
            Main.terminal.Printf("%X ", b);
        }

        Main.terminal.Puts("\n");

        Write(address, toWrite);
    }
}
