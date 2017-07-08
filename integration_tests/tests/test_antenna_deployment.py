from system import auto_power_on, runlevel, clear_state
from datetime import timedelta

from tests.base import BaseTest
from utils import TestEvent


class Test_AntennaDeployment(BaseTest):
    @auto_power_on(False)
    def __init__(self, methodName='runTest'):
        super(Test_AntennaDeployment, self).__init__(methodName)

    def begin_deployment(self):
        self.system.obc.jump_to_time(timedelta(minutes=42))

    def next_step(self):
        self.system.obc.run_mission()

    def run_steps(self, count):
        while count > 0:
            self.next_step()
            count -= 1

    def begin(self, count=1):
        self.power_on_obc()
        self.begin_deployment()
        self.run_steps(count)

    @runlevel(1)
    @clear_state()
    def test_antennas_are_not_deployed_too_soon(self):
        event = TestEvent()
        self.system.primary_antenna.on_begin_deployment = event.set
        self.power_on_obc()
        self.system.obc.jump_to_time(20 * 60)
        self.next_step()
        self.assertFalse(event.wait_for_change(1), "antenna deployment process began too soon")

    @runlevel(1)
    @clear_state()
    def test_process_begins_automatically_by_controller_reset(self):
        event = TestEvent()
        self.system.primary_antenna.on_reset = event.set
        self.begin(1)
        self.assertTrue(event.wait_for_change(1), "antenna controller was not reset")

    @runlevel(1)
    @clear_state()
    def test_process_begins_automatically(self):
        event = TestEvent()
        self.system.primary_antenna.on_begin_deployment = event.set
        self.begin(2)
        self.assertTrue(event.wait_for_change(1), "antenna deployment process did not began when it should")

    @runlevel(1)
    @clear_state()
    def test_deployment_is_cancelled_on_retry(self):
        event = TestEvent()
        self.system.primary_antenna.on_deployment_cancel = event.set
        self.begin(2)
        self.assertTrue(event.wait_for_change(1), "antenna deployment process was not cancelled")

    @runlevel(1)
    @clear_state()
    def test_deployment_arming_sequences(self):
        primaryBegin = TestEvent()
        primaryEnd = TestEvent()
        backupBegin = TestEvent()
        backupEnd = TestEvent()

        def primaryHandler(newState):
            if newState:
                primaryBegin.set()
            else:
                primaryEnd.set()

        def backupHandler(newState):
            if newState:
                backupBegin.set()
            else:
                backupEnd.set()

        def deployemntHandler(*args):
            return False

        self.system.primary_antenna.on_arm_state_change = primaryHandler
        self.system.backup_antenna.on_arm_state_change = backupHandler
        self.system.primary_antenna.on_begin_deployment = deployemntHandler
        self.system.backup_antenna.on_begin_deployment = deployemntHandler
        self.begin(14)
        self.assertTrue(primaryBegin.wait_for_change(1), "primary controller was not armed")
        self.assertTrue(primaryEnd.wait_for_change(1), "primary controller was not disarmed")
        self.assertTrue(backupBegin.wait_for_change(1), "backup controller was not armed")
        self.assertTrue(backupEnd.wait_for_change(1), "backup controller was not disarmed")

    @runlevel(1)
    @clear_state()
    def test_all_antennas_are_deployed_manually(self):
        list = [0, 0, 0, 0, 0, 0, 0, 0, 0]
        expected = [0, 1, 1, 1, 1, 1, 1, 1, 1]

        def primaryHandler(controller, antennaId):
            if antennaId != -1:
                list[antennaId] = 1
            return False

        def backupHandler(controller, antennaId):
            if antennaId != -1:
                list[antennaId + 4] = 1
            return False

        self.system.primary_antenna.on_begin_deployment = primaryHandler
        self.system.backup_antenna.on_begin_deployment = backupHandler
        self.begin(14)
        self.assertSequenceEqual(list, expected)
