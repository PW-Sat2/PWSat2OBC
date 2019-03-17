#include "flash_eraser.hpp"
#include "base/os.h"
#include "state.hpp"

NullErrorCounter ErrorCounter;

FlashEraser::FlashEraser(SPIPeripheral& spi)                           //
    : _flash1Spi(SPIDevice<io_map::SlaveSelectFlash1>(spi)),           //
      _flash2Spi(SPIDevice<io_map::SlaveSelectFlash2>(spi)),           //
      _flash3Spi(SPIDevice<io_map::SlaveSelectFlash3>(spi)),           //
      _flash1(devices::n25q::N25QDriver(ErrorCounter, 1, _flash1Spi)), //
      _flash2(devices::n25q::N25QDriver(ErrorCounter, 1, _flash2Spi)), //
      _flash3(devices::n25q::N25QDriver(ErrorCounter, 1, _flash3Spi))  //
{
}

void FlashEraser::Initialize()
{
    this->_flash1Spi.Initialize();
    this->_flash2Spi.Initialize();
    this->_flash3Spi.Initialize();

    this->_flash1.Reset();
    this->_flash2.Reset();
    this->_flash3.Reset();
}

void FlashEraser::Run()
{
    auto wait1 = _flash1.BeginEraseChip();
    auto wait2 = _flash2.BeginEraseChip();
    auto wait3 = _flash3.BeginEraseChip();

    //  Watchdog is 256 seconds, we put 60 seconds wait it allow erase operation to finish.
    //  It takes typically 100 seconds, MAX 240 seconds in the datasheet, but we don't have that time.
    //  On our platforms, it takes about 40 seconds.

    for (int i = 0; i < 10 * 60; ++i)
    {
        extern void wait_100ms_spinloop();
        wait_100ms_spinloop();
    }

    //    We make no use of following flags.
    //    We ignore them.

    //    wait1.Wait();
    //    wait2.Wait();
    //    wait3.Wait();

    wait1.Cancel();
    wait2.Cancel();
    wait3.Cancel();
}
