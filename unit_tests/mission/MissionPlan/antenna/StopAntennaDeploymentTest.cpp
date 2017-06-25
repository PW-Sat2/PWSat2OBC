#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "mission/antenna_task.hpp"
#include "mission/base.hpp"
#include "state/antenna/AntennaConfiguration.hpp"
#include "state/struct.h"

using testing::Eq;

namespace
{
    class StopAntennaDeploymentTest : public testing::Test
    {
      protected:
        SystemState _state;
        mission::antenna::StopAntennaDeploymentTask _task{0};
        mission::ActionDescriptor<SystemState> _action{_task.BuildAction()};
    };

    TEST_F(StopAntennaDeploymentTest, DoesNothingIfNotOrderedTo)
    {
        auto shouldExecute = _action.EvaluateCondition(this->_state);

        ASSERT_THAT(shouldExecute, Eq(false));
    }

    TEST_F(StopAntennaDeploymentTest, ShouldNotExecuteIfOrderedToStopButDeployingAlreadyDisabled)
    {
        this->_task.DisableDeployment();

        _state.PersistentState.Set<state::AntennaConfiguration>(state::AntennaConfiguration(true));

        auto shouldExecute = _action.EvaluateCondition(this->_state);

        ASSERT_THAT(shouldExecute, Eq(false));
    }

    TEST_F(StopAntennaDeploymentTest, ShouldExecuteIfOrderToStopDeployingAndNotDisabledYet)
    {
        this->_state.AntennaState.SetDeployment(true);

        this->_task.DisableDeployment();

        auto shouldExecute = _action.EvaluateCondition(this->_state);

        ASSERT_THAT(shouldExecute, Eq(true));

        _action.Execute(this->_state);

        auto a = _state.PersistentState.Get<state::AntennaConfiguration>();
        ASSERT_THAT(a.IsDeploymentDisabled(), Eq(true));
    }

    TEST_F(StopAntennaDeploymentTest, ShouldDoNothingIfOrderToStopDeploymentButItIsStillInProgress)
    {
        this->_state.AntennaState.SetDeployment(false);

        this->_task.DisableDeployment();

        auto shouldExecute = _action.EvaluateCondition(this->_state);

        ASSERT_THAT(shouldExecute, Eq(false));
    }

    TEST_F(StopAntennaDeploymentTest, ShouldDelayDisablingDeplymentToTheEndOfDeploymentProcedure)
    {
        this->_state.AntennaState.SetDeployment(false);

        this->_task.DisableDeployment();

        auto shouldExecute = _action.EvaluateCondition(this->_state);

        ASSERT_THAT(shouldExecute, Eq(false));

        this->_state.AntennaState.SetDeployment(true);

        shouldExecute = _action.EvaluateCondition(this->_state);

        ASSERT_THAT(shouldExecute, Eq(true));
    }
}
