#include "commands.h"
#include "obc.h"

using devices::eps::LCL;
using devices::eps::BurnSwitch;

static void Usage()
{
    Main.terminal.Puts("eps enable_lcl|disable_lcl");
}

static void Print(const char* prefix, devices::eps::hk::HouseheepingControllerA::MPPT_HK& hk)
{
    Main.terminal.Printf("%s.SOL_VOLT\t%d\n", prefix, hk.SOL_VOLT.Value());
    Main.terminal.Printf("%s.SOL_CURR\t%d\n", prefix, hk.SOL_CURR.Value());
    Main.terminal.Printf("%s.SOL_OUT_VOLT\t%d\n", prefix, hk.SOL_OUT_VOLT.Value());
    Main.terminal.Printf("%s.TEMP\t%d\n", prefix, hk.TEMP.Value());
    Main.terminal.Printf("%s.STATE\t%d\n", prefix, num(hk.STATE));
}

static void Print(const char* prefix, devices::eps::hk::HouseheepingControllerA::DISTR_HK& hk)
{
    Main.terminal.Printf("%s.CURR_3V3\t%d\n", prefix, hk.CURR_3V3.Value());
    Main.terminal.Printf("%s.VOLT_3V3\t%d\n", prefix, hk.VOLT_3V3.Value());
    Main.terminal.Printf("%s.CURR_5V\t%d\n", prefix, hk.CURR_5V.Value());
    Main.terminal.Printf("%s.VOLT_5V\t%d\n", prefix, hk.VOLT_5V.Value());
    Main.terminal.Printf("%s.CURR_VBAT\t%d\n", prefix, hk.CURR_VBAT.Value());
    Main.terminal.Printf("%s.VOLT_VBAT\t%d\n", prefix, hk.VOLT_VBAT.Value());
    Main.terminal.Printf("%s.TEMP\t%d\n", prefix, hk.TEMP.Value());
    Main.terminal.Printf("%s.LCL_STATE\t%d\n", prefix, num(hk.LCL_STATE));
    Main.terminal.Printf("%s.LCL_FLAGB\t%d\n", prefix, num(hk.LCL_FLAGB));
}

static void Print(const char* prefix, devices::eps::hk::HouseheepingControllerA::BATC_HK& hk)
{
    Main.terminal.Printf("%s.VOLT_A\t%d\n", prefix, hk.VOLT_A.Value());
    Main.terminal.Printf("%s.CHRG_CURR\t%d\n", prefix, hk.CHRG_CURR.Value());
    Main.terminal.Printf("%s.DCHRG_CURR\t%d\n", prefix, hk.DCHRG_CURR.Value());
    Main.terminal.Printf("%s.TEMP\t%d\n", prefix, hk.TEMP.Value());
    Main.terminal.Printf("%s.STATE\t%d\n", prefix, num(hk.STATE));
}

static void Print(const char* prefix, devices::eps::hk::HouseheepingControllerB::BATC_HK& hk)
{
    Main.terminal.Printf("%s.VOLT_B\t%d\n", prefix, hk.VOLT_B.Value());
}

static void Print(const char* prefix, devices::eps::hk::HouseheepingControllerA::BP_HK& hk)
{
    Main.terminal.Printf("%s.TEMP_A\t%d\n", prefix, hk.TEMP_A.Value());
    Main.terminal.Printf("%s.TEMP_B\t%d\n", prefix, hk.TEMP_B.Value());
}

static void Print(const char* prefix, devices::eps::hk::HouseheepingControllerB::BP_HK& hk)
{
    Main.terminal.Printf("%s.TEMP_C\t%d\n", prefix, hk.TEMP_C.Value());
}

static void Print(const char* prefix, devices::eps::hk::OtherController& hk)
{
    Main.terminal.Printf("%s.VOLT_3V3d\t%d\n", prefix, hk.VOLT_3V3d.Value());
}

static void Print(const char* prefix, devices::eps::hk::ThisController& hk)
{
    Main.terminal.Printf("%s.ERR\t%d\n", prefix, hk.ERR);
    Main.terminal.Printf("%s.PWR_CYCLES\t%d\n", prefix, hk.PWR_CYCLES);
    Main.terminal.Printf("%s.UPTIME\t%ld\n", prefix, hk.UPTIME);
    Main.terminal.Printf("%s.TEMP\t%d\n", prefix, hk.TEMP.Value());
}

static void Print(const char* prefix, devices::eps::hk::HouseheepingControllerA::DCDC_HK& hk)
{
    Main.terminal.Printf("%s.TEMP\t%d\n", prefix, hk.TEMP.Value());
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
            result = eps.PowerCycleA();
        }

        else if (strcmp(argv[1], "B") == 0)
        {
            result = eps.PowerCycleB();
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
            result = eps.DisableOverheatSubmodeA();
        }

        else if (strcmp(argv[1], "B") == 0)
        {
            result = eps.DisableOverheatSubmodeB();
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

        Print("MPPT_X", v.MPPT_X);
        Print("MPPT_Y_PLUS", v.MPPT_Y_PLUS);
        Print("MPPT_Y_MINUS", v.MPPT_Y_MINUS);
        Print("DISTR", v.DISTR);
        Print("BATC", v.BATC);
        Print("BP", v.BP);
        Print("CTRLB", v.CTRLB);
        Print("CTRLA", v.CTRLA);
        Print("DCDC3V3", v.DCDC3V3);
        Print("DCDC5V", v.DCDC5V);

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

        Print("BP", v.BP);
        Print("BATC", v.BATC);
        Print("CTRLA", v.CTRLA);
        Print("CTRLB", v.CTRLB);

        return;
    }
}
