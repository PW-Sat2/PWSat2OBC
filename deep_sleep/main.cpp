#include <stdio.h>
#include <string.h>
#include <em_acmp.h>
#include <em_burtc.h>
#include <em_chip.h>
#include <em_cmu.h>
#include <em_emu.h>
#include <em_i2c.h>
#include <em_rmu.h>
#include <em_usart.h>
#include "base/reader.h"
#include "base/writer.h"
#include "comm.hpp"
#include "config.hpp"
#include "eps.hpp"
#include "flash_eraser.hpp"
#include "logger/logger.h"
#include "mcu/io_map.h"
#include "obc/memory.hpp"
#include "scrubbing.hpp"
#include "sleep.h"
#include "standalone/i2c/i2c.hpp"
#include "standalone/spi/spi.hpp"
#include "state.hpp"
#include "system.h"
#include "timer.h"

#include "boot/params.hpp"

using namespace std::chrono;

void SendToUart(USART_TypeDef* uart, const char* message)
{
    while (*message != '\0')
    {
        USART_Tx(uart, *message);
        message++;
    }
}

// static void LogToUart(void* context, bool /*withinIsr*/, const char* messageHeader, const char* messageFormat, va_list messageArguments)
// {
//     char buf[256];
//     auto uart = static_cast<USART_TypeDef*>(context);
//     SendToUart(uart, messageHeader);
//     vsprintf(buf, messageFormat, messageArguments);
//     SendToUart(uart, buf);
//     USART_Tx(uart, '\n');
// }

extern "C" void __libc_init_array(void);

StandaloneI2C PayloadI2C(I2C1);
StandaloneI2C BusI2C(I2C0);
StandaloneEPS EPS(BusI2C, PayloadI2C);
StandaloneComm Comm(BusI2C);
obc::OBCMemory Memory;

drivers::msc::MCUMemoryController MCUFlash;
StandaloneFlashDriver FlashDriver(io_map::ProgramFlash::FlashBase);
program_flash::BootTable BootTable(FlashDriver);

SPIPeripheral Spi;
State PersistentState{Spi};
FlashEraser Eraser{Spi};

using PLDI2C = io_map::I2C_1;

constexpr std::uint8_t Gyro = 0x68;

static bool RamAlreadyLatched = false;

void ACMP0_IRQHandler()
{
    SendToUart(io_map::UART_1::Peripheral, "L");
    if (RamAlreadyLatched)
    {
        while (1)
        {
            EPS.PowerCycle(EPSController::A);
            Sleep(1s);
            EPS.PowerCycle(EPSController::B);
            Sleep(1s);
        }
    }

    if (has_flag(ACMP_IntGet(ACMP0), ACMP_IF_EDGE))
    {
        Memory.HandleLatchup(obc::MemoryModule::SRAM1);
        ACMP_IntClear(ACMP0, ACMP_IFC_EDGE);
    }

    if (has_flag(ACMP_IntGet(ACMP1), ACMP_IF_EDGE))
    {
        Memory.HandleLatchup(obc::MemoryModule::SRAM2);
        ACMP_IntClear(ACMP1, ACMP_IFC_EDGE);
    }

    RamAlreadyLatched = true;
}

static void InitI2C()
{
    using pld = io_map::I2C_1;
    using bus = io_map::I2C_0;

    PayloadI2C.Initialize(cmuClock_I2C1, pld::SDA::Port, pld::SDA::PinNumber, pld::SCL::Port, pld::SCL::PinNumber, pld::Location);

    BusI2C.Initialize(cmuClock_I2C0, bus::SDA::Port, bus::SDA::PinNumber, bus::SCL::Port, bus::SCL::PinNumber, bus::Location);
}

static void GyroSleep()
{
    SendToUart(io_map::UART_1::Peripheral, "GS");

    {
        std::array<uint8_t, 2> cmd = {0x3E, 1 << 6};
        const auto status = PayloadI2C.Write(Gyro, cmd);
        if (status == drivers::i2c::I2CResult::OK)
        {
        }
        else
        {
            SendToUart(io_map::UART_1::Peripheral, "N\n");
        }
    }
}

static void InitObcWatchdog()
{
    WDOG_Init_TypeDef init = WDOG_INIT_DEFAULT;
    init.debugRun = false;
    init.enable = true;
    init.perSel = wdogPeriod_256k;

    WDOGn_Init(WDOG, &init);
}

static void DisableLCLs()
{
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

static void RebootToDeepSleep(std::uint32_t swap)
{
    while (1)
    {
        EPS.PowerCycle(swap ? EPSController::A : EPSController::B);
        Sleep(1s);
        EPS.PowerCycle(swap ? EPSController::B : EPSController::A);
        Sleep(1s);
    }
}

static void RebootToNormal()
{
    PersistentState.SwapBootSlots();
    while (1)
    {
        EPS.PowerCycle(EPSController::A);
        Sleep(1s);
        EPS.PowerCycle(EPSController::B);
        Sleep(1s);
    }
}

static void EraseFlash(void*, const Counter&)
{
    SendToUart(io_map::UART_1::Peripheral, "EF\n");

    Eraser.Run();
}

static void DoRebootToNormal(void*, const Counter&)
{
    SendToUart(io_map::UART_1::Peripheral, "RN");
    RebootToNormal();
}

void SetupHardware(void)
{
    CMU_ClockEnable(cmuClock_GPIO, true);

    // LFXTAL
    GPIO_PinModeSet(io_map::XTAL::LF::Pin1::Port, io_map::XTAL::LF::Pin1::PinNumber, gpioModeDisabled, 0);
    GPIO_PinModeSet(io_map::XTAL::LF::Pin2::Port, io_map::XTAL::LF::Pin2::PinNumber, gpioModeDisabled, 0);

    // HFXTAL
    GPIO_PinModeSet(io_map::XTAL::HF::Pin1::Port, io_map::XTAL::HF::Pin1::PinNumber, gpioModeDisabled, 0);
    GPIO_PinModeSet(io_map::XTAL::HF::Pin2::Port, io_map::XTAL::HF::Pin2::PinNumber, gpioModeDisabled, 0);

    CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_HFCLKLE);
    CMU_ClockSelectSet(cmuClock_LFB, cmuSelect_HFCLKLE);

    CMU_OscillatorEnable(cmuOsc_HFXO, true, true);
    CMU_ClockSelectSet(cmuClock_HF, cmuSelect_HFXO);
    CMU_OscillatorEnable(cmuOsc_HFRCO, false, true);
}

static constexpr std::array<std::uint8_t, 3> BeaconHeader = {0x24, 0, 0};

void SendBeacon(
    const EPSTelemetryA& epsA, const EPSTelemetryB& epsB, std::chrono::milliseconds currentTime, std::int32_t rebootToNormalValue)
{
    Comm.SetTransmitterBitRate(COMM::Bitrate::Comm9600bps);
    SendToUart(io_map::UART_1::Peripheral, "B");
    std::array<std::uint8_t, 3 + 4 + 2 + 2 + 4> beaconBuffer;
    Reader aReader{epsA.Buffer};
    Reader bReader{epsB.Buffer};
    Writer writer{beaconBuffer};
    writer.WriteArray(gsl::make_span(BeaconHeader));
    writer.WriteDoubleWordLE(currentTime.count());
    aReader.Skip(42);
    writer.WriteWordLE(aReader.ReadWordLE());
    bReader.Skip(3);
    writer.WriteWordLE(bReader.ReadWordLE());
    writer.WriteDoubleWordLE(rebootToNormalValue);
    Comm.SendFrame(gsl::make_span(beaconBuffer));
}

int main()
{
    SCB->VTOR = 0x00080000;

    CHIP_Init();

    SetupHardware();

    __libc_init_array();

    {
        USART_InitAsync_TypeDef init = USART_INITASYNC_DEFAULT;
        init.baudrate = io_map::UART_1::Baudrate;
        init.enable = usartDisable;
        CMU_ClockEnable(cmuClock_UART1, true);
        USART_InitAsync(io_map::UART_1::Peripheral, &init);
    }

    io_map::UART_1::Peripheral->ROUTE |= UART_ROUTE_TXPEN | io_map::UART_1::Location;

    // SystickIndicator
    GPIO_PinModeSet(io_map::SystickIndicator::Port, io_map::SystickIndicator::PinNumber, gpioModePushPull, 0);
    // BootIndicator
    GPIO_PinModeSet(io_map::BootIndicator::Port, io_map::BootIndicator::PinNumber, gpioModePushPull, 0);
    // TimeIndicator
    GPIO_PinModeSet(io_map::TimeIndicator::Port, io_map::TimeIndicator::PinNumber, gpioModePushPull, 0);

    InitI2C();

    USART_Enable(io_map::UART_1::Peripheral, usartEnable);

    USART_Tx(io_map::UART_1::Peripheral, '!');

    InitObcWatchdog();

    GyroSleep();
    DisableLCLs();

    ConfigureBurtc();

    DWT_Init();

    milliseconds next_scrubbing = 0ms;
    milliseconds nextBeacon = 0ms;

    Spi.Initialize();
    PersistentState.Initialize();
    FlashDriver.Initialize();
    BootTable.Initialize();
    Eraser.Initialize();

    Counter eraseFlashCounter{CounterType::EraseFlash, Config::EraseFlashCycles, EraseFlash, const_cast<char*>("FE.\n")};
    Counter rebootToNormalCounter{CounterType::RebootToNormal, Config::RebootToNormalAfter, DoRebootToNormal, const_cast<char*>("RN.\n")};

    eraseFlashCounter.Verify(PersistentState);
    rebootToNormalCounter.Verify(PersistentState);

    char msg[256];
    auto rebootToNormalValue = PersistentState.ReadCounter(CounterType::RebootToNormal);
    SendToUart(io_map::UART_1::Peripheral, "toNormal=");
    itoa(rebootToNormalValue, msg, 10);
    SendToUart(io_map::UART_1::Peripheral, msg);
    SendToUart(io_map::UART_1::Peripheral, "\n");

    PersistentState.ConfirmBoot();

    while (1)
    {
        auto current_time = GetTime();

        // Deep-sleep logic goes here
        SendToUart(io_map::UART_1::Peripheral, "MS=");
        itoa((uint32_t)current_time.count(), msg, 10);
        SendToUart(io_map::UART_1::Peripheral, msg);
        SendToUart(io_map::UART_1::Peripheral, "\n");

        if (GetTime() >= Config::RebootToDeepSleepThreshold)
        {
            RebootToDeepSleep(PersistentState.BootCounter() & 1);
        }

        EPSTelemetryA epsA;
        EPSTelemetryB epsB;
        EPS.ReadTelemetryA(epsA);
        EPS.ReadTelemetryB(epsB);
        EPS.KickWatchdogs();

        WDOGn_Feed(WDOG);

        // Reset Comm watchdogs and check if there are frames
        auto rxFrameStatus = Comm.PollHardware();
        if (rxFrameStatus == StandaloneFrameType::Reboot)
        {
            SendToUart(io_map::UART_1::Peripheral, "FR\n");
            rebootToNormalCounter.Reset(PersistentState);
            RebootToNormal();
        }
        else if (rxFrameStatus == StandaloneFrameType::SendBeacon)
        {
            nextBeacon = GetTime();
        }

        if (current_time >= nextBeacon)
        {
            SendBeacon(epsA, epsB, current_time, rebootToNormalValue);
            nextBeacon += Config::BeaconInterval;
        }

        if (current_time >= next_scrubbing)
        {
            next_scrubbing = current_time + Config::ScrubbingInterval;

            SendToUart(io_map::UART_1::Peripheral, "SCB");

            ScrubProgram(MCUFlash, FlashDriver, BootTable);

            SendToUart(io_map::UART_1::Peripheral, ".");
        }

        // Setup next BURTC iteration
        ArmBurtc();

        SendToUart(io_map::UART_1::Peripheral, "S\n");
        while (!(io_map::UART_1::Peripheral->STATUS & USART_STATUS_TXC))
            ;

        while (EBI->STATUS & EBI_STATUS_AHBACT)
        {
        }
        EMU_EnterEM1();

        // SendToUart(io_map::UART_1::Peripheral, "Wake up!\n");
        UpdateTime();
    }
}
