import devices
from tests.base import BaseTest
from system import auto_comm_handling
from threading import Event

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

    def test_arming_deployment_system(self):
        event = TestEvent()
        def handler(newState):
            if newState:
                event.set()

        self.system.primary_antenna.on_arm_state_change = handler
        self.power_on_and_wait()
        self.system.obc.antenna_arm_deployment()
        self.assertTrue(event.wait_for_change(1))

    def test_disarming_deployment_system(self):
        event = TestEvent()
        def handler(newState):
            if not newState:
                event.set()

        self.system.primary_antenna.on_arm_state_change = handler
        self.power_on_and_wait()
        self.system.obc.antenna_arm_deployment()
        self.system.obc.antenna_disarm_deployment()
        self.assertTrue(event.wait_for_change(1))

    def test_auto_deployment(self):
        event = TestEvent()
        def handler(driver, antenna):
            if antenna == -1:
                event.set()

        self.system.primary_antenna.on_begin_deployment = handler
        self.power_on_and_wait()
        self.system.obc.antenna_autodeploy()
        self.assertTrue(event.wait_for_change(1))
