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
        self.system.obc.antenna_deploy(AntennaChannel.Primary, AntennaId.Antenna2, False)
        self.assertTrue(event.wait_for_change(1))

    def test_manual_deployment_with_override(self):
        event = TestEvent()
        def handler(driver, antenna):
            if antenna == 2:
                event.set()

        self.system.backup_antenna.on_begin_deployment = handler
        self.power_on_and_wait()
        result = self.system.obc.antenna_deploy(AntennaChannel.Backup, AntennaId.Antenna2, True)
        self.assertTrue(event.wait_for_change(1))
        self.assertTrue(self.system.backup_antenna.ignore_deployment_switch)

    def test_deployment_finalization(self):
        event = TestEvent()
        self.system.backup_antenna.on_deployment_cancel = event.set
        self.power_on_and_wait()
        result = self.system.obc.antenna_cancel_deployment(AntennaChannel.Backup)
        self.assertTrue(event.wait_for_change(1))
