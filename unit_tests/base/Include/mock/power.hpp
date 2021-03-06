#ifndef UNIT_TESTS_BASE_MOCK_POWER_HPP_
#define UNIT_TESTS_BASE_MOCK_POWER_HPP_

#include "gmock/gmock.h"
#include "power/power.h"

struct PowerControlMock : services::power::IPowerControl
{
    PowerControlMock();
    ~PowerControlMock();

    MOCK_METHOD0(PowerCycle, void());
    MOCK_METHOD0(OpenSail, void());
    MOCK_METHOD1(MainThermalKnife, bool(bool));
    MOCK_METHOD1(RedundantThermalKnife, bool(bool));
    MOCK_METHOD0(EnableMainSailBurnSwitch, bool());
    MOCK_METHOD0(EnableRedundantSailBurnSwitch, bool());
    MOCK_METHOD0(EnableMainSADSBurnSwitch, bool());
    MOCK_METHOD0(EnableRedundantSADSBurnSwitch, bool());

    MOCK_METHOD1(PrimaryAntennaPower, bool(bool enable));
    MOCK_METHOD1(BackupAntennaPower, bool(bool enable));

    MOCK_METHOD0(PrimaryAntennaPower, Option<bool>());

    MOCK_METHOD1(SensPower, bool(bool enable));
    MOCK_METHOD1(SunSPower, bool(bool enable));

    MOCK_METHOD1(CameraWing, bool(bool enable));
    MOCK_METHOD1(CameraNadir, bool(bool enable));

    MOCK_METHOD1(ImtqPower, bool(bool enable));

    MOCK_METHOD0(IgnoreOverheat, bool());
};

#endif /* UNIT_TESTS_BASE_MOCK_POWER_HPP_ */
