#include "eps/eps.h"
#include <cstdlib>
#include <cstring>
#include "obc_access.hpp"
#include "terminal/terminal.h"

using devices::eps::LCL;
using devices::eps::BurnSwitch;
using devices::eps::EPSDriver;

static void Usage()
{
    GetTerminal().Puts("eps enable_lcl|disable_lcl|power_cycle|disable_overheat|enable_burn_switch|hk_a|hk_b");
}

static void Print(const char* prefix, const devices::eps::hk::MPPT_HK& hk)
{
    GetTerminal().Printf("%s.SOL_VOLT\t%d\n", prefix, hk.SOL_VOLT.Value());
    GetTerminal().Printf("%s.SOL_CURR\t%d\n", prefix, hk.SOL_CURR.Value());
    GetTerminal().Printf("%s.SOL_OUT_VOLT\t%d\n", prefix, hk.SOL_OUT_VOLT.Value());
    GetTerminal().Printf("%s.TEMP\t%d\n", prefix, hk.Temperature.Value());
    GetTerminal().Printf("%s.STATE\t%d\n", prefix, num(hk.MpptState));
}

static void Print(const char* prefix, const devices::eps::hk::DISTR_HK& hk)
{
    GetTerminal().Printf("%s.CURR_3V3\t%d\n", prefix, hk.CURR_3V3.Value());
    GetTerminal().Printf("%s.VOLT_3V3\t%d\n", prefix, hk.VOLT_3V3.Value());
    GetTerminal().Printf("%s.CURR_5V\t%d\n", prefix, hk.CURR_5V.Value());
    GetTerminal().Printf("%s.VOLT_5V\t%d\n", prefix, hk.VOLT_5V.Value());
    GetTerminal().Printf("%s.CURR_VBAT\t%d\n", prefix, hk.CURR_VBAT.Value());
    GetTerminal().Printf("%s.VOLT_VBAT\t%d\n", prefix, hk.VOLT_VBAT.Value());
    GetTerminal().Printf("%s.LCL_STATE\t%d\n", prefix, num(hk.LCL_STATE));
    GetTerminal().Printf("%s.LCL_FLAGB\t%d\n", prefix, num(hk.LCL_FLAGB));
}

static void Print(const char* prefix, const devices::eps::hk::BATCPrimaryState& hk)
{
    GetTerminal().Printf("%s.VOLT_A\t%d\n", prefix, hk.VOLT_A.Value());
    GetTerminal().Printf("%s.CHRG_CURR\t%d\n", prefix, hk.ChargeCurrent.Value());
    GetTerminal().Printf("%s.DCHRG_CURR\t%d\n", prefix, hk.DischargeCurrent.Value());
    GetTerminal().Printf("%s.TEMP\t%d\n", prefix, hk.Temperature.Value());
    GetTerminal().Printf("%s.STATE\t%d\n", prefix, num(hk.State));
}

static void Print(const char* prefix, const devices::eps::hk::BATCSecondaryState& hk)
{
    GetTerminal().Printf("%s.VOLT_B\t%d\n", prefix, hk.voltB.Value());
}

static void Print(const char* prefix, const devices::eps::hk::BatteryPackPrimaryState hk)
{
    GetTerminal().Printf("%s.TEMP_A\t%d\n", prefix, hk.temperatureA.Value());
    GetTerminal().Printf("%s.TEMP_B\t%d\n", prefix, hk.temperatureB.Value());
}

static void Print(const char* prefix, const devices::eps::hk::BatteryPackSecondaryState& hk)
{
    GetTerminal().Printf("%s.TEMP_C\t%d\n", prefix, hk.temperatureC.Value());
}

static void Print(const char* prefix, const devices::eps::hk::OtherControllerState& hk)
{
    GetTerminal().Printf("%s.VOLT_3V3d\t%d\n", prefix, hk.VOLT_3V3d.Value());
}

static void Print(const char* prefix, const devices::eps::hk::ThisControllerState& hk)
{
    GetTerminal().Printf("%s.SAFETY_CTR\t%d\n", prefix, hk.safetyCounter);
    GetTerminal().Printf("%s.PWR_CYCLES\t%d\n", prefix, hk.powerCycleCount);
    GetTerminal().Printf("%s.UPTIME\t%ld\n", prefix, hk.uptime);
    GetTerminal().Printf("%s.TEMP\t%d\n", prefix, hk.temperature.Value());
    GetTerminal().Printf("%s.SUPP_TEMP\t%d\n", prefix, hk.suppTemp.Value());
}

static void Print(const char* prefix, devices::eps::hk::DCDC_HK& hk)
{
    GetTerminal().Printf("%s.TEMP\t%d\n", prefix, hk.temperature.Value());
}

void EPSCommand(std::uint16_t argc, char* argv[])
{
    if (argc == 0)
    {
        Usage();
        return;
    }

    auto& eps = GetEPS();

    if (strcmp(argv[0], "enable_lcl") == 0)
    {
        auto lcl = static_cast<LCL>(atoi(argv[1]));
        auto result = eps.EnableLCL(lcl);

        GetTerminal().Printf("%d", num(result));
        return;
    }

    if (strcmp(argv[0], "disable_lcl") == 0)
    {
        auto lcl = static_cast<LCL>(atoi(argv[1]));
        auto result = eps.DisableLCL(lcl);

        GetTerminal().Printf("%d", num(result));
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

        GetTerminal().Puts(result ? "1" : "0");
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

        GetTerminal().Puts(result ? "1" : "0");
        return;
    }

    if (strcmp(argv[0], "enable_burn_switch") == 0)
    {
        bool useMain = strcmp(argv[1], "1") == 0;

        auto burnSwitch = static_cast<BurnSwitch>(atoi(argv[2]));

        auto result = eps.EnableBurnSwitch(useMain, burnSwitch);

        GetTerminal().Printf("%d", num(result));
        return;
    }

    if (strcmp(argv[0], "hk_a") == 0)
    {
        auto hk = eps.ReadHousekeepingA();
        if (!hk.HasValue)
        {
            GetTerminal().Puts("Error");
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
            GetTerminal().Puts("Error");
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

        GetTerminal().Puts(result ? "1" : "0");
        return;
    }
}
