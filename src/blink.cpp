#include "base/os.h"
#include "obc.h"

using namespace std::chrono_literals;

enum class TimeLed
{
    ConstantOn,
    FastBlink,
    SlowBlink,
    ConstantOff
};

TimeLed TimeLedMode(std::chrono::milliseconds currentTime)
{
    if (currentTime > 12min)
    {
        return TimeLed::ConstantOff;
    }

    if (currentTime > 8min)
    {
        return TimeLed::FastBlink;
    }

    if (currentTime > 4min)
    {
        return TimeLed::SlowBlink;
    }

    return TimeLed::ConstantOn;
}

static void TaskProc(void* /*param*/)
{
    TimeLed timeLed = TimeLed::ConstantOn;

    auto lastTickBlink = System::GetUptime();

    while (true)
    {
        auto currentTime = Main.timeProvider.GetCurrentTime();

        if (currentTime.HasValue)
        {
            timeLed = TimeLedMode(currentTime.Value);
        }

        if (timeLed == TimeLed::ConstantOn)
        {
            Main.Hardware.Pins.TimeIndicator.High();
            System::SleepTask(1s);
        }
        else if (timeLed == TimeLed::ConstantOff)
        {
            Main.Hardware.Pins.TimeIndicator.Low();
            System::SleepTask(1s);
        }
        else if (timeLed == TimeLed::FastBlink)
        {
            Main.Hardware.Pins.TimeIndicator.Toggle();
            System::SleepTask(250ms);
        }
        else if (timeLed == TimeLed::SlowBlink)
        {
            Main.Hardware.Pins.TimeIndicator.Toggle();
            System::SleepTask(1s);
        }

        if (System::GetUptime() - lastTickBlink >= 1s)
        {
            Main.Hardware.Pins.SystickIndicator.Toggle();
            lastTickBlink = System::GetUptime();
        }
    }
}

static Task<void*, 1_KB, TaskPriority::P1> BlinkTask("Blink", nullptr, TaskProc);

void InitializeBlink()
{
    BlinkTask.Create();
}
