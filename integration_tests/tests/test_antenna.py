import devices
from tests.base import BaseTest
from system import auto_comm_handling
from threading import Event

class TestEvent():
    flag = Event()

    def set(self):
        self.flag.set()

    def reset(self):
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
