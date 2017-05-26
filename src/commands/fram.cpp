#include <array>
#include "base/writer.h"
#include "commands.h"
#include "fm25w/fm25w.hpp"
#include "obc.h"

using std::uint8_t;
using devices::fm25w::FM25WDriver;
using devices::fm25w::IFM25WDriver;
using devices::fm25w::RedundantFM25WDriver;
using devices::fm25w::Status;
using drivers::spi::EFMSPISlaveInterface;

static void Status(IFM25WDriver& fram)
{
    auto sr = fram.ReadStatus();

    Main.terminal.Printf("Status register=%X\n", num(sr.Value));
}

static void Write(IFM25WDriver& fram, std::uint16_t address, gsl::span<const std::uint8_t> value)
{
    fram.Write(address, value);

    Status(fram);
}

static void Read(IFM25WDriver& fram, std::uint16_t address, gsl::span<uint8_t> value)
{
    fram.Read(address, value);
}

static void WriteSingleFRAM(devices::fm25w::IFM25WDriver& fram)
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
}

static bool ReadSingleFRAM(devices::fm25w::IFM25WDriver& fram)
{
    std::uint16_t address = 5;

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

static void FillSingleFRAM(devices::fm25w::IFM25WDriver& fram, uint8_t value)
{
    std::uint16_t address = 5;

    {
        std::array<std::uint8_t, 16> writeBuffer;
        writeBuffer.fill(value);

        auto toWrite = gsl::make_span(writeBuffer);

        fram.Write(address, toWrite);
    }
}

static bool TestSingleFRAM(devices::fm25w::IFM25WDriver& fram)
{
    WriteSingleFRAM(fram);
    return ReadSingleFRAM(fram);
}

static IFM25WDriver& ParseFRAMIndex(char index, std::array<IFM25WDriver*, 3>& drivers, RedundantFM25WDriver& redundantDriver)
{
    if (index >= '0' && index < '3')
        return *drivers[static_cast<uint8_t>(index - '0')];

    return redundantDriver;
}

static bool IsFRAMIndexValid(char index)
{
    return index == 'r' || (index >= '0' && index < '3');
}

void FRAM(std::uint16_t argc, char* argv[])
{
    if (argc == 0)
    {
        Main.terminal.Puts("fram <status|write|read|testall|tmr>\n");
        return;
    }

    auto& fram1 = Main.Hardware.PersistentStorage.GetSingleDriver<0>();
    auto& fram2 = Main.Hardware.PersistentStorage.GetSingleDriver<1>();
    auto& fram3 = Main.Hardware.PersistentStorage.GetSingleDriver<2>();

    auto& redundantDriver = Main.Hardware.PersistentStorage.GetRedundantDriver();

    std::array<devices::fm25w::IFM25WDriver*, 3> drivers{&fram1, &fram2, &fram3};

    if (strcmp(argv[0], "status") == 0)
    {
        if (!IsFRAMIndexValid(argv[1][0]))
        {
            Main.terminal.Puts("Invalid fram\n");
            return;
        }

        auto& fram = ParseFRAMIndex(argv[1][0], drivers, redundantDriver);
        Status(fram);
    }

    if (strcmp(argv[0], "read") == 0)
    {
        if (!IsFRAMIndexValid(argv[1][0]))
        {
            Main.terminal.Puts("Invalid fram\n");
            return;
        }

        std::uint16_t address = atoi(argv[2]);
        std::array<std::uint8_t, 16> buf;

        auto toRead = gsl::make_span(buf).subspan(0, atoi(argv[3]));

        auto& fram = ParseFRAMIndex(argv[1][0], drivers, redundantDriver);
        Read(fram, address, toRead);

        for (auto b : toRead)
        {
            Main.terminal.Printf("%X ", b);
        }
    }

    if (strcmp(argv[0], "write") == 0)
    {
        if (!IsFRAMIndexValid(argv[1][0]))
        {
            Main.terminal.Puts("Invalid fram\n");
            return;
        }

        std::uint16_t address = atoi(argv[2]);
        std::array<std::uint8_t, 16> buf;

        for (auto i = 3; i < argc; i++)
        {
            buf[i - 3] = atoi(argv[i]);
        }

        auto toWrite = gsl::make_span(buf).subspan(0, argc - 3);

        for (auto b : toWrite)
        {
            Main.terminal.Printf("%X ", b);
        }

        Main.terminal.Puts("\n");

        auto& fram = ParseFRAMIndex(argv[1][0], drivers, redundantDriver);

        Write(fram, address, toWrite);
    }

    if (strcmp(argv[0], "testall") == 0)
    {
        if (strcmp(argv[1], "f") != 0)
        {
            Main.terminal.Printf("This operation writes to all flashes. Add \"f\" parameter to proceed.");
            return;
        }

        auto isOk1 = TestSingleFRAM(fram1);
        auto isOk2 = TestSingleFRAM(fram2);
        auto isOk3 = TestSingleFRAM(fram3);

        Main.terminal.Printf("Fram 1 read write ok: %d\r\n", isOk1);
        Main.terminal.Printf("Fram 2 read write ok: %d\r\n", isOk2);
        Main.terminal.Printf("Fram 3 read write ok: %d\r\n", isOk3);

        if (!isOk1 || !isOk2 || !isOk3)
        {
            Main.terminal.Printf("SOME FRAMS ARE INVALID!\r\n");
        }
    }

    if (strcmp(argv[0], "tmr") == 0)
    {
        if (strcmp(argv[1], "f") != 0)
        {
            Main.terminal.Printf("This operation writes to all flashes. Add \"f\" parameter to proceed.");
            return;
        }

        Main.terminal.Printf("Starting single FRAM failing test\r\n");

        for (uint8_t failingIndex = 0; failingIndex < 3; ++failingIndex)
        {
            for (uint8_t i = 0; i < 3; ++i)
            {
                if (i == failingIndex)
                {
                    FillSingleFRAM(*drivers[i], 1);
                }
                else
                {
                    WriteSingleFRAM(*drivers[i]);
                }
            }

            auto isRedundantOk = ReadSingleFRAM(redundantDriver);
            Main.terminal.Printf("Redundant read ok: %d\r\n", isRedundantOk);
        }

        Main.terminal.Printf("Starting double FRAM failing test\r\n");

        for (uint8_t failingIndex = 0; failingIndex < 3; ++failingIndex)
        {
            for (uint8_t i = 0; i < 3; ++i)
            {
                if (i != failingIndex)
                {
                    FillSingleFRAM(*drivers[i], 1);
                }
                else
                {
                    WriteSingleFRAM(*drivers[i]);
                }
            }

            auto isRedundantOk = !ReadSingleFRAM(redundantDriver);
            Main.terminal.Printf("Redundant read ok: %d\r\n", isRedundantOk);
        }
    }
}
