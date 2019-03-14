#include <stdio.h>
#include <string.h>
#include <em_burtc.h>
#include <em_chip.h>
#include <em_cmu.h>
#include <em_emu.h>
#include <em_i2c.h>
#include <em_rmu.h>
#include <em_usart.h>
#include "mcu/io_map.h"
#include "standalone/i2c/i2c.hpp"
#include "system.h"
#include "timer.h"

#include "boot/params.hpp"

static void SendToUart(USART_TypeDef* uart, const char* message)
{
    while (*message != '\0')
    {
        USART_Tx(uart, *message);
        message++;
    }
}

extern "C" void __libc_init_array(void);

StandaloneI2C PayloadI2C(I2C1);

using PLDI2C = io_map::I2C_1;

constexpr std::uint8_t Gyro = 0x68;

static void InitI2C()
{
    PayloadI2C.Initialize(
        cmuClock_I2C1, PLDI2C::SDA::Port, PLDI2C::SDA::PinNumber, PLDI2C::SCL::Port, PLDI2C::SCL::PinNumber, PLDI2C::Location);
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

int main()
{
    SCB->VTOR = 0x00080000;

    CHIP_Init();

    __libc_init_array();

    CMU_ClockEnable(cmuClock_GPIO, true);

    USART_InitAsync_TypeDef init = USART_INITASYNC_DEFAULT;
    init.baudrate = io_map::UART_1::Baudrate;
    init.enable = usartDisable;

    CMU_ClockEnable(cmuClock_UART1, true);
    USART_InitAsync(io_map::UART_1::Peripheral, &init);

    io_map::UART_1::Peripheral->ROUTE |= UART_ROUTE_TXPEN | io_map::UART_1::Location;

    InitI2C();

    USART_Enable(io_map::UART_1::Peripheral, usartEnable);

    USART_Tx(io_map::UART_1::Peripheral, '!');

    char msg[256] = {0};

    sprintf(msg, "Magic: 0x%lX\nReason=%d\nIndex=%d\n", boot::MagicNumber, num(boot::BootReason), boot::Index);
    SendToUart(io_map::UART_1::Peripheral, msg);

    GyroSleep();

    ConfigureBurtc();
    SendToUart(io_map::UART_1::Peripheral, "Configured Burtc!\n");

    while (1)
    {
        // Deep-sleep logic goes here        
        sprintf(msg, "Time ms=%lu\n", (uint32_t)GetTime().count());
        SendToUart(io_map::UART_1::Peripheral, msg);

        // Setup next BURTC iteration
        ArmBurtc();

        SendToUart(io_map::UART_1::Peripheral, "Sleeping!\n");
        while (!(io_map::UART_1::Peripheral->STATUS & USART_STATUS_TXC))
            ;
        EMU_EnterEM3(true);

        SendToUart(io_map::UART_1::Peripheral, "Wake up!\n");
    }
}