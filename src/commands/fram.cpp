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
    EFMSPISlaveInterface spi(Main.Hardware.SPI, Main.Hardware.Pins.Fram1ChipSelect);

    FM25WDriver fram(spi);

    auto sr = fram.ReadStatus();

    Main.terminal.Printf("Status register=%X\n", num(sr));
}

static void Write(std::uint16_t address, gsl::span<const std::uint8_t> value)
{
    EFMSPISlaveInterface spi(Main.Hardware.SPI, Main.Hardware.Pins.Fram1ChipSelect);

    FM25WDriver fram(spi);

    fram.Write(address, value);

    Status();
}

static void Read(std::uint16_t address, gsl::span<uint8_t> value)
{
    EFMSPISlaveInterface spi(Main.Hardware.SPI, Main.Hardware.Pins.Fram1ChipSelect);

    FM25WDriver fram(spi);
    fram.Read(address, value);
}

static bool TestSingleFRAM(devices::fm25w::FM25WDriver& fram)
{
    std::uint16_t address = 5;

    {
        std::array<std::uint8_t, 16> writeBuffer;

        for (auto i = 0; i < 16; i++)
        {
            writeBuffer[i] = i;
        }

        auto toWrite = gsl::make_span(writeBuffer);

        fram.Write(address, toWrite);
    }

    {
        std::array<std::uint8_t, 16> readBuffer;
        fram.Read(address, readBuffer);

        bool isOk = true;
        for (auto i = 0; i < 16; i++)
        {
            if (readBuffer[i] != i)
            {
                isOk = false;
            }
        }

        return isOk;
    }
}

void FRAM(std::uint16_t argc, char* argv[])
{
    if (argc == 0)
    {
        Main.terminal.Puts("fram <status|write|read|testall>\n");
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

    if (strcmp(argv[0], "testall") == 0)
    {
        if (strcmp(argv[1], "f") != 0)
        {
            Main.terminal.Printf("This operation writes to all flashes. Add \"f\" parameter to proceed.");
            return;
        }

        drivers::spi::EFMSPISlaveInterface spi1(Main.Hardware.SPI, Main.Hardware.Pins.Fram1ChipSelect);
        drivers::spi::EFMSPISlaveInterface spi2(Main.Hardware.SPI, Main.Hardware.Pins.Fram2ChipSelect);
        drivers::spi::EFMSPISlaveInterface spi3(Main.Hardware.SPI, Main.Hardware.Pins.Fram3ChipSelect);

        devices::fm25w::FM25WDriver fram1(spi1);
        devices::fm25w::FM25WDriver fram2(spi2);
        devices::fm25w::FM25WDriver fram3(spi3);

        auto isOk1 = TestSingleFRAM(fram1);
        auto isOk2 = TestSingleFRAM(fram2);
        auto isOk3 = TestSingleFRAM(fram3);

        Main.terminal.Printf("Fram 1 read write ok: %d\r\n", isOk1);
        Main.terminal.Printf("Fram 2 read write ok: %d\r\n", isOk2);
        Main.terminal.Printf("Fram 3 read write ok: %d\r\n", isOk3);

        if (!isOk1 || !isOk2 || !isOk3)
        {
            Main.terminal.Printf("SOME FRAMS ARE INVALID!");
        }
    }
}
