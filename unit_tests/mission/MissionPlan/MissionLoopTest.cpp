#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"
#include "OsMock.hpp"
#include "mission/main.hpp"
#include "mock/ActionDescriptorMock.hpp"
#include "mock/UpdateDescriptorMock.hpp"
#include "mock/VerifyDescriprorMock.hpp"
#include "os/os.hpp"

using testing::Return;
using testing::Eq;
using testing::_;
using namespace mission;

namespace
{
    struct State
    {
    };

    typedef MissionLoop<State,
        ActionDescriptorMock<State, void>,
        UpdateDescriptorMock<State, void>,
        ActionDescriptorMock<State, int>,
        VerifyDescriptorMock<State, void>,
        VerifyDescriptorMock<State, int>,
        VerifyDescriptorMock<State, float>>
        Mission;

    struct MissionLoopTest : public testing::Test
    {
        Mission mission;
        testing::StrictMock<OSMock> mock;
    };

    TEST_F(MissionLoopTest, TestInitializationFailure)
    {
        auto proxy = InstallProxy(&mock);
        EXPECT_CALL(mock, CreateEventGroup()).WillOnce(Return(nullptr));
        const auto status = mission.Initialize(10s);
        ASSERT_THAT(status, Eq(false));
    }

    TEST_F(MissionLoopTest, TestTaskCreationFailure)
    {
        auto proxy = InstallProxy(&mock);
        EXPECT_CALL(mock, CreateEventGroup()).WillOnce(Return(this));
        EXPECT_CALL(mock, CreateTask(_, _, _, _, _, _)).WillOnce(Return(OSResult::IOError));
        const auto status = mission.Initialize(10s);
        ASSERT_THAT(status, Eq(false));
    }

    TEST_F(MissionLoopTest, TestInitialization)
    {
        auto proxy = InstallProxy(&mock);
        EXPECT_CALL(mock, CreateEventGroup()).WillOnce(Return(this));
        EXPECT_CALL(mock, CreateTask(_, _, _, _, _, _)).WillOnce(Return(OSResult::Success));
        EXPECT_CALL(mock, SuspendTask(_));
        const auto status = mission.Initialize(10s);
        ASSERT_THAT(status, Eq(true));
    }

    TEST_F(MissionLoopTest, TestLoopBodyUpdateDescriptors)
    {
        auto& update1 = static_cast<UpdateDescriptorMock<State, void>&>(mission);
        EXPECT_CALL(update1, UpdateProc(_)).WillOnce(Return(UpdateResult::Ok));
        mission.RunOnce();
    }

    TEST_F(MissionLoopTest, TestLoopBodyVerifyDescriptors)
    {
        auto& verify1 = static_cast<VerifyDescriptorMock<State, void>&>(mission);
        auto& verify2 = static_cast<VerifyDescriptorMock<State, int>&>(mission);
        auto& verify3 = static_cast<VerifyDescriptorMock<State, float>&>(mission);
        EXPECT_CALL(verify1, VerifyProc(_)).WillOnce(Return(VerifyDescriptorResult(VerifyResult::Ok, 0)));
        EXPECT_CALL(verify2, VerifyProc(_)).WillOnce(Return(VerifyDescriptorResult(VerifyResult::Ok, 0)));
        EXPECT_CALL(verify3, VerifyProc(_)).WillOnce(Return(VerifyDescriptorResult(VerifyResult::Ok, 0)));
        mission.RunOnce();
    }

    TEST_F(MissionLoopTest, TestLoopBodyActionDescriptors)
    {
        auto& action1 = static_cast<ActionDescriptorMock<State, void>&>(mission);
        auto& action2 = static_cast<ActionDescriptorMock<State, int>&>(mission);
        EXPECT_CALL(action1, ConditionProc(_)).WillOnce(Return(false));
        EXPECT_CALL(action2, ConditionProc(_)).WillOnce(Return(true));
        EXPECT_CALL(action2, ActionProc(_)).Times(1);
        mission.RunOnce();
    }
}
