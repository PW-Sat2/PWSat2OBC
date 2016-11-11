import devices
from tests.base import BaseTest
from system import auto_comm_handling
from threading import Event
from obc import *

class TestEvent():
    flag = Event()

    def set(self, *args):
        self.flag.set()

    def reset(self, *args):
        self.flag.clear();

    def wait_for_change(self, timeout = None):
        return self.flag.wait(timeout)

class Test_Antenna(BaseTest):
    def __init__(self, methodName = 'runTest'):
        result = super(Test_Antenna, self).__init__(methodName)
        self.auto_power_on = False
        return result

    def test_primary_antenna_is_reset_at_startup(self):
        event = TestEvent();
        self.system.primary_antenna.on_reset = event.set
        self.system.obc.power_on()
        self.assertTrue(event.wait_for_change(1))

    def test_backup_antenna_is_reset_at_startup(self):
        event = TestEvent();
        self.system.backup_antenna.on_reset = event.set
        self.system.obc.power_on()
        self.assertTrue(event.wait_for_change(1))

    def test_auto_deployment(self):
        event = TestEvent()
        def handler(driver, antenna):
            if antenna == -1:
                event.set()

        self.system.primary_antenna.on_begin_deployment = handler
        self.power_on_and_wait()
        self.system.obc.antenna_deploy(AntennaChannel.Primary, AntennaId.Auto, OverrideSwitches.Disabled)
        self.assertTrue(event.wait_for_change(1))

    def test_manual_deployment(self):
        event = TestEvent()
        def handler(driver, antenna):
            if antenna == 2:
                event.set()

        self.system.primary_antenna.on_begin_deployment = handler
        self.power_on_and_wait()
        self.system.obc.antenna_deploy(AntennaChannel.Primary, AntennaId.Antenna2, OverrideSwitches.Disabled)
        self.assertTrue(event.wait_for_change(1))

    def test_manual_deployment_with_override(self):
        event = TestEvent()
        def handler(driver, antenna):
            if antenna == 2:
                event.set()

        self.system.backup_antenna.on_begin_deployment = handler
        self.power_on_and_wait()
        result = self.system.obc.antenna_deploy(AntennaChannel.Backup, AntennaId.Antenna2, OverrideSwitches.Enabled)
        self.assertTrue(event.wait_for_change(1))
        self.assertTrue(self.system.backup_antenna.ignore_deployment_switch)

    def test_deployment_finalization(self):
        event = TestEvent()
        self.system.backup_antenna.on_deployment_cancel = event.set
        self.power_on_and_wait()
        result = self.system.obc.antenna_cancel_deployment(AntennaChannel.Backup)
        self.assertTrue(event.wait_for_change(1))

    def test_telemetry(self):
        def return_state():
            return [0x0f, 0xe0]

        self.system.backup_antenna.on_get_deployment_status = return_state
        self.power_on_and_wait()
        result = self.system.obc.antenna_get_status(AntennaChannel.Backup)
        self.assertTrue(result.Status)
        self.assertTrue(result.SystemArmed)
        self.assertFalse(result.IgnoringSwitches)

        self.assertFalse(result.DeploymentState[0])
        self.assertTrue(result.DeploymentState[1])
        self.assertTrue(result.DeploymentState[2])
        self.assertFalse(result.DeploymentState[3])

        self.assertTrue(result.DeploymentInProgress[0])
        self.assertFalse(result.DeploymentInProgress[1])
        self.assertFalse(result.DeploymentInProgress[2])
        self.assertTrue(result.DeploymentInProgress[3])

