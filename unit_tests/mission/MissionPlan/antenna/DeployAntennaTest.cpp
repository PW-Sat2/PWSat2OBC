#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "OsMock.hpp"
#include "mission/antenna_task.hpp"
#include "mission/base.hpp"
#include "mock/AntennaMock.hpp"
#include "mock/power.hpp"
#include "state/struct.h"

using namespace mission;
using namespace mission::antenna;
using namespace std::chrono_literals;
using testing::_;
using testing::InSequence;
using testing::DoAll;
using testing::Assign;
using testing::Return;
using testing::Eq;

namespace
{
    class DeployAntennaTest : public testing::Test
    {
      protected:
        DeployAntennaTest();

        std::chrono::milliseconds _nextTime;
        SystemState _state;

        testing::StrictMock<PowerControlMock> _power;
        testing::StrictMock<AntennaMock> _antenna;

        testing::NiceMock<OSMock> _os;
        OSReset _osReset{InstallProxy(&_os)};

        AntennaTask _task{std::make_tuple(std::ref(_antenna), std::ref(_power))};
        ActionDescriptor<SystemState> _action{_task.BuildAction()};

        void Run(std::uint32_t maxIterations);
    };

    DeployAntennaTest::DeployAntennaTest()
    {
        ON_CALL(_os, TakeSemaphore(_, _)).WillByDefault(Return(OSResult::Success));
        ON_CALL(_os, GiveSemaphore(_)).WillByDefault(Return(OSResult::Success));
    }

    void DeployAntennaTest::Run(std::uint32_t maxIterations)
    {
        for (auto i = 0U; i < maxIterations; i++)
        {
            if (!_action.EvaluateCondition(_state))
            {
                return;
            }

            _action.Execute(_state);

            if (_nextTime > _state.Time)
            {
                ASSERT_THAT(_action.EvaluateCondition(_state), Eq(false)) << "Iteration " << i << ": should wait for time "
                                                                          << _nextTime.count() << "ms";
                _state.Time = _nextTime;
            }
        }

        ASSERT_TRUE(false) << "Task should stop executing in less than " << maxIterations << " iterations";
    }

    TEST_F(DeployAntennaTest, HappyPath)
    {
        _state.Time = 40min;

        {
            InSequence s;

            auto t = _state.Time;

            auto expectDeployment = [&](
                AntennaChannel channel, AntennaId antenna, std::chrono::milliseconds burnTime, std::chrono::milliseconds waitTime) {

                if (channel == AntennaChannel::ANTENNA_PRIMARY_CHANNEL)
                {
                    EXPECT_CALL(_power, PrimaryAntennaPower(true)).WillOnce(DoAll(Assign(&_nextTime, t += 10s), Return(true)));
                }
                else
                {
                    EXPECT_CALL(_power, BackupAntennaPower(true)).WillOnce(DoAll(Assign(&_nextTime, t += 10s), Return(true)));
                }

                EXPECT_CALL(_antenna, Reset(channel)).WillOnce(DoAll(Assign(&_nextTime, t += 60s), Return(OSResult::Success)));

                EXPECT_CALL(_antenna, Arm(channel)).WillOnce(DoAll(Assign(&_nextTime, t += 60s), Return(OSResult::Success)));

                EXPECT_CALL(_antenna, DeployAntenna(channel, antenna, burnTime, true))
                    .WillOnce(DoAll(Assign(&_nextTime, t += waitTime), Return(OSResult::Success)));

                EXPECT_CALL(_antenna, Disarm(channel)).WillOnce(DoAll(Assign(&_nextTime, t += 0s), Return(OSResult::Success)));

                if (channel == AntennaChannel::ANTENNA_PRIMARY_CHANNEL)
                {
                    EXPECT_CALL(_power, PrimaryAntennaPower(false)).WillOnce(DoAll(Assign(&_nextTime, t += 120s), Return(true)));
                }
                else
                {
                    EXPECT_CALL(_power, BackupAntennaPower(false)).WillOnce(DoAll(Assign(&_nextTime, t += 120s), Return(true)));
                }
            };

            expectDeployment(AntennaChannel::ANTENNA_PRIMARY_CHANNEL, AntennaId::ANTENNA_AUTO_ID, 120000ms, 180s);
            expectDeployment(AntennaChannel::ANTENNA_PRIMARY_CHANNEL, AntennaId::ANTENNA1_ID, 30000ms, 90s);
            expectDeployment(AntennaChannel::ANTENNA_PRIMARY_CHANNEL, AntennaId::ANTENNA2_ID, 30000ms, 90s);
            expectDeployment(AntennaChannel::ANTENNA_PRIMARY_CHANNEL, AntennaId::ANTENNA3_ID, 30000ms, 90s);
            expectDeployment(AntennaChannel::ANTENNA_PRIMARY_CHANNEL, AntennaId::ANTENNA4_ID, 30000ms, 90s);

            expectDeployment(AntennaChannel::ANTENNA_BACKUP_CHANNEL, AntennaId::ANTENNA_AUTO_ID, 120000ms, 180s);
            expectDeployment(AntennaChannel::ANTENNA_BACKUP_CHANNEL, AntennaId::ANTENNA1_ID, 30000ms, 90s);
            expectDeployment(AntennaChannel::ANTENNA_BACKUP_CHANNEL, AntennaId::ANTENNA2_ID, 30000ms, 90s);
            expectDeployment(AntennaChannel::ANTENNA_BACKUP_CHANNEL, AntennaId::ANTENNA3_ID, 30000ms, 90s);
            expectDeployment(AntennaChannel::ANTENNA_BACKUP_CHANNEL, AntennaId::ANTENNA4_ID, 30000ms, 90s);
        }

        Run(100);

        ASSERT_THAT(_state.AntennaState.IsDeployed(), Eq(true));
    }

    TEST_F(DeployAntennaTest, ShouldNotOpenAntennaBeforeSilentPeriodIsOver)
    {
        _state.Time = 25min;

        ASSERT_THAT(_action.EvaluateCondition(_state), Eq(false));
    }

    TEST_F(DeployAntennaTest, ShouldNotOpenAntennaIfOpeningIsDisabled)
    {
        _state.Time = 2h;

        _state.PersistentState.Set(state::AntennaConfiguration(true));

        ASSERT_THAT(_action.EvaluateCondition(_state), Eq(false));
    }
}
