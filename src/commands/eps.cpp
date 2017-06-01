#include "commands.h"
#include "obc.h"

using devices::eps::LCL;
using devices::eps::BurnSwitch;
using devices::eps::EPSDriver;

static void Usage()
{
    Main.terminal.Puts("eps enable_lcl|disable_lcl|power_cycle|disable_overheat|enable_burn_switch|hk_a|hk_b");
}

static void Print(const char* prefix, const devices::eps::hk::MPPT_HK& hk)
{
    Main.terminal.Printf("%s.SOL_VOLT\t%d\n", prefix, hk.SOL_VOLT.Value());
    Main.terminal.Printf("%s.SOL_CURR\t%d\n", prefix, hk.SOL_CURR.Value());
    Main.terminal.Printf("%s.SOL_OUT_VOLT\t%d\n", prefix, hk.SOL_OUT_VOLT.Value());
    Main.terminal.Printf("%s.TEMP\t%d\n", prefix, hk.Temperature.Value());
    Main.terminal.Printf("%s.STATE\t%d\n", prefix, num(hk.MpptState));
}

static void Print(const char* prefix, const devices::eps::hk::DISTR_HK& hk)
{
    Main.terminal.Printf("%s.CURR_3V3\t%d\n", prefix, hk.CURR_3V3.Value());
    Main.terminal.Printf("%s.VOLT_3V3\t%d\n", prefix, hk.VOLT_3V3.Value());
    Main.terminal.Printf("%s.CURR_5V\t%d\n", prefix, hk.CURR_5V.Value());
    Main.terminal.Printf("%s.VOLT_5V\t%d\n", prefix, hk.VOLT_5V.Value());
    Main.terminal.Printf("%s.CURR_VBAT\t%d\n", prefix, hk.CURR_VBAT.Value());
    Main.terminal.Printf("%s.VOLT_VBAT\t%d\n", prefix, hk.VOLT_VBAT.Value());
    Main.terminal.Printf("%s.TEMP\t%d\n", prefix, hk.Temperature.Value());
    Main.terminal.Printf("%s.LCL_STATE\t%d\n", prefix, num(hk.LCL_STATE));
    Main.terminal.Printf("%s.LCL_FLAGB\t%d\n", prefix, num(hk.LCL_FLAGB));
}

static void Print(const char* prefix, const devices::eps::hk::BATCPrimaryState& hk)
{
    Main.terminal.Printf("%s.VOLT_A\t%d\n", prefix, hk.VOLT_A.Value());
    Main.terminal.Printf("%s.CHRG_CURR\t%d\n", prefix, hk.ChargeCurrent.Value());
    Main.terminal.Printf("%s.DCHRG_CURR\t%d\n", prefix, hk.DischargeCurrent.Value());
    Main.terminal.Printf("%s.TEMP\t%d\n", prefix, hk.Temperature.Value());
    Main.terminal.Printf("%s.STATE\t%d\n", prefix, num(hk.State));
}

static void Print(const char* prefix, const devices::eps::hk::BATCSecondaryState& hk)
{
    Main.terminal.Printf("%s.VOLT_B\t%d\n", prefix, hk.voltB.Value());
}

static void Print(const char* prefix, const devices::eps::hk::BatteryPackPrimaryState hk)
{
    Main.terminal.Printf("%s.TEMP_A\t%d\n", prefix, hk.temperatureA.Value());
    Main.terminal.Printf("%s.TEMP_B\t%d\n", prefix, hk.temperatureB.Value());
}

static void Print(const char* prefix, const devices::eps::hk::BatteryPackSecondaryState& hk)
{
    Main.terminal.Printf("%s.TEMP_C\t%d\n", prefix, hk.temperatureC.Value());
}

static void Print(const char* prefix, const devices::eps::hk::OtherControllerState& hk)
{
    Main.terminal.Printf("%s.VOLT_3V3d\t%d\n", prefix, hk.VOLT_3V3d.Value());
}

static void Print(const char* prefix, const devices::eps::hk::ThisControllerState& hk)
{
    Main.terminal.Printf("%s.ERR\t%d\n", prefix, hk.errorCode);
    Main.terminal.Printf("%s.PWR_CYCLES\t%d\n", prefix, hk.powerCycleCount);
    Main.terminal.Printf("%s.UPTIME\t%ld\n", prefix, hk.uptime);
    Main.terminal.Printf("%s.TEMP\t%d\n", prefix, hk.temperature.Value());
}

static void Print(const char* prefix, devices::eps::hk::DCDC_HK& hk)
{
    Main.terminal.Printf("%s.TEMP\t%d\n", prefix, hk.temperature.Value());
}

void EPSCommand(std::uint16_t argc, char* argv[])
{
    if (argc == 0)
    {
        Usage();
        return;
    }

    auto& eps = Main.Hardware.EPS;

    if (strcmp(argv[0], "enable_lcl") == 0)
    {
        auto lcl = static_cast<LCL>(atoi(argv[1]));
        auto result = eps.EnableLCL(lcl);

        Main.terminal.Printf("%d", num(result));
        return;
    }

    if (strcmp(argv[0], "disable_lcl") == 0)
    {
        auto lcl = static_cast<LCL>(atoi(argv[1]));
        auto result = eps.DisableLCL(lcl);

        Main.terminal.Printf("%d", num(result));
        return;
    }

    if (strcmp(argv[0], "power_cycle") == 0)
    {
        bool result = false;

        if (strcmp(argv[1], "A") == 0)
        {
            result = eps.PowerCycle(EPSDriver::Controller::A);
        }

        else if (strcmp(argv[1], "B") == 0)
        {
            result = eps.PowerCycle(EPSDriver::Controller::B);
        }
        else if (strcmp(argv[1], "Auto") == 0)
        {
            result = eps.PowerCycle();
        }

        Main.terminal.Puts(result ? "1" : "0");
        return;
    }

    if (strcmp(argv[0], "disable_overheat") == 0)
    {
        bool result = false;

        if (strcmp(argv[1], "A") == 0)
        {
            result = eps.DisableOverheatSubmode(EPSDriver::Controller::A);
        }

        else if (strcmp(argv[1], "B") == 0)
        {
            result = eps.DisableOverheatSubmode(EPSDriver::Controller::B);
        }

        Main.terminal.Puts(result ? "1" : "0");
        return;
    }

    if (strcmp(argv[0], "enable_burn_switch") == 0)
    {
        bool useMain = strcmp(argv[1], "1") == 0;

        auto burnSwitch = static_cast<BurnSwitch>(atoi(argv[2]));

        auto result = eps.EnableBurnSwitch(useMain, burnSwitch);

        Main.terminal.Printf("%d", num(result));
        return;
    }

    if (strcmp(argv[0], "hk_a") == 0)
    {
        auto hk = eps.ReadHousekeepingA();
        if (!hk.HasValue)
        {
            Main.terminal.Puts("Error");
            return;
        }

        auto v = hk.Value;

        Print("MPPT_X", v.mpptX);
        Print("MPPT_Y_PLUS", v.mpptYPlus);
        Print("MPPT_Y_MINUS", v.mpptYMinus);
        Print("DISTR", v.distr);
        Print("BATC", v.batc);
        Print("BP", v.bp);
        Print("CTRLB", v.other);
        Print("CTRLA", v.current);
        Print("DCDC3V3", v.dcdc3V3);
        Print("DCDC5V", v.dcdc5V);

        return;
    }

    if (strcmp(argv[0], "hk_b") == 0)
    {
        auto hk = eps.ReadHousekeepingB();
        if (!hk.HasValue)
        {
            Main.terminal.Puts("Error");
            return;
        }

        auto v = hk.Value;

        Print("BP", v.bp);
        Print("BATC", v.batc);
        Print("CTRLA", v.other);
        Print("CTRLB", v.current);

        return;
    }

    if (strcmp(argv[0], "reset_watchdog") == 0)
    {
        bool result = false;

        if (strcmp(argv[1], "A") == 0)
        {
            result = eps.ResetWatchdog(EPSDriver::Controller::A) == devices::eps::ErrorCode::NoError;
        }

        else if (strcmp(argv[1], "B") == 0)
        {
            result = eps.ResetWatchdog(EPSDriver::Controller::B) == devices::eps::ErrorCode::NoError;
        }
        else if (strcmp(argv[1], "Both") == 0)
        {
            result = eps.ResetWatchdog(EPSDriver::Controller::A) == devices::eps::ErrorCode::NoError;
            result = result && eps.ResetWatchdog(EPSDriver::Controller::B) == devices::eps::ErrorCode::NoError;
        }

        Main.terminal.Puts(result ? "1" : "0");
        return;
    }
}
