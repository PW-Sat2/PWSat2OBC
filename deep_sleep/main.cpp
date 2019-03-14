#include <stdio.h>
#include <string.h>
#include <em_burtc.h>
#include <em_chip.h>
#include <em_cmu.h>
#include <em_emu.h>
#include <em_i2c.h>
#include <em_rmu.h>
#include <em_usart.h>
#include "eps.hpp"
#include "mcu/io_map.h"
#include "standalone/i2c/i2c.hpp"
#include "standalone/spi/spi.hpp"
#include "state.hpp"
#include "system.h"
#include "timer.h"
#include "flash_eraser.hpp"

#include "boot/params.hpp"

void SendToUart(USART_TypeDef* uart, const char* message)
{
    while (*message != '\0')
    {
        USART_Tx(uart, *message);
        message++;
    }
}

extern "C" void __libc_init_array(void);

StandaloneI2C PayloadI2C(I2C1);
StandaloneI2C BusI2C(I2C0);
StandaloneEPS EPS(BusI2C, PayloadI2C);

SPIPeripheral Spi;
State PersistentState{Spi};
FlashEraser Eraser{Spi};

using PLDI2C = io_map::I2C_1;

constexpr std::uint8_t Gyro = 0x68;

static void InitI2C()
{
    using pld = io_map::I2C_1;
    using bus = io_map::I2C_0;

    PayloadI2C.Initialize(cmuClock_I2C1, pld::SDA::Port, pld::SDA::PinNumber, pld::SCL::Port, pld::SCL::PinNumber, pld::Location);

    BusI2C.Initialize(cmuClock_I2C0, bus::SDA::Port, bus::SDA::PinNumber, bus::SCL::Port, bus::SCL::PinNumber, bus::Location);
}

static void GyroSleep()
{
    SendToUart(io_map::UART_1::Peripheral, "Gyro sleep\n");

    {
        std::array<uint8_t, 1> inData = {static_cast<uint8_t>(0x00)};
        std::array<uint8_t, 1> outData;

        auto status = PayloadI2C.WriteRead(0x68, inData, outData);

        if (status == drivers::i2c::I2CResult::OK && outData[0] == 0x68)
        {
            SendToUart(io_map::UART_1::Peripheral, "Gyro ID ok\n");
        }
        else
        {
            SendToUart(io_map::UART_1::Peripheral, "Gyro ID nok\n");
        }
    }

    {
        std::array<uint8_t, 2> cmd = {0x3E, 1 << 6};
        const auto status = PayloadI2C.Write(Gyro, cmd);
        if (status == drivers::i2c::I2CResult::OK)
        {
            SendToUart(io_map::UART_1::Peripheral, "Gyro Sleep ok\n");
        }
        else
        {
            SendToUart(io_map::UART_1::Peripheral, "Gyro Sleep nok\n");
        }
    }
}

static void DisableLCLs()
{
    SendToUart(io_map::UART_1::Peripheral, "Disabling LCLs\n");

    EPS.DisableLCL(LCL::TKMain);
    EPS.DisableLCL(LCL::SunS);
    EPS.DisableLCL(LCL::CamNadir);
    EPS.DisableLCL(LCL::CamWing);
    EPS.DisableLCL(LCL::SENS);
    EPS.DisableLCL(LCL::AntennaMain);
    EPS.DisableLCL(LCL::IMTQ);
    EPS.DisableLCL(LCL::TKRed);
    EPS.DisableLCL(LCL::AntennaRed);
}

static void BootPrinter(void* text, const Counter&)
{
    SendToUart(io_map::UART_1::Peripheral, static_cast<const char*>(text));
}

static void EraseFlash(void*, const Counter&)
{
    SendToUart(io_map::UART_1::Peripheral, "Erasing flash\n");

    Eraser.Run();
}

int main()
{
    SCB->VTOR = 0x00080000;

    CHIP_Init();

    __libc_init_array();

    CMU_ClockEnable(cmuClock_GPIO, true);
    {
        USART_InitAsync_TypeDef init = USART_INITASYNC_DEFAULT;
        init.baudrate = io_map::UART_1::Baudrate;
        init.enable = usartDisable;
        CMU_ClockEnable(cmuClock_UART1, true);
        USART_InitAsync(io_map::UART_1::Peripheral, &init);
    }

    io_map::UART_1::Peripheral->ROUTE |= UART_ROUTE_TXPEN | io_map::UART_1::Location;

    InitI2C();

    USART_Enable(io_map::UART_1::Peripheral, usartEnable);

    USART_Tx(io_map::UART_1::Peripheral, '!');

    char msg[256] = {0};

    sprintf(msg, "Magic: 0x%lX\nReason=%d\nIndex=%d\n", boot::MagicNumber, num(boot::BootReason), boot::Index);
    SendToUart(io_map::UART_1::Peripheral, msg);

    GyroSleep();
    DisableLCLs();

    ConfigureBurtc();
    SendToUart(io_map::UART_1::Peripheral, "Configured Burtc!\n");

    Spi.Initialize();
    PersistentState.Initialize();
    Eraser.Initialize();

    Counter printCounter{CounterType::PrintCounter, 1, BootPrinter, const_cast<char*>("Boot Action done\n")};
    Counter eraseFlashCounter{CounterType::EraseFlash, 10, EraseFlash, const_cast<char*>("Flash erased\n")};

    printCounter.Verify(PersistentState);
    eraseFlashCounter.Verify(PersistentState);

    while (1)
    {
        // Deep-sleep logic goes here
        sprintf(msg, "Time ms=%lu\n", (uint32_t)GetTime().count());
        SendToUart(io_map::UART_1::Peripheral, msg);

        EPSTelemetryA epsA;
        EPSTelemetryB epsB;
        EPS.ReadTelemetryA(epsA);
        EPS.ReadTelemetryB(epsB);

        // Setup next BURTC iteration
        ArmBurtc();

        SendToUart(io_map::UART_1::Peripheral, "Sleeping!\n");
        while (!(io_map::UART_1::Peripheral->STATUS & USART_STATUS_TXC))
            ;
        EMU_EnterEM3(true);

        SendToUart(io_map::UART_1::Peripheral, "Wake up!\n");
    }
}