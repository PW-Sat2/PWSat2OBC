import devices
from tests.base import BaseTest
from obc import *
from utils import TestEvent
from system import auto_power_on

class Test_Beacon(BaseTest):
    @auto_power_on(False)
    def __init__(self, methodName = 'runTest'):
        return super(Test_AntennaDeployment, self).__init__(methodName)

    def begin_deployment(self):
        self.system.obc.jump_to_time(42 * 60)

    def next_step(self):
        self.system.obc.run_mission()

    def run_steps(self, count):
        while count > 0:
            self.next_step()
            count -= 1

    def begin(self, count = 1):
        self.power_on_and_wait()
        self.system.obc.suspend_mission()
        self.begin_deployment()
        self.run_steps(count)

    def test_beacon_auto_activation(self):
        event = TestEvent()
        def reset_handler(*args):
            return False

        self.system.primary_antenna.on_reset = reset_handler
        self.system.primary_antenna.finish_deployment()
        self.system.comm.transmitter.on_set_beacon = event.set
        self.begin(10)
        self.assertTrue(event.wait_for_change(1), "beacon should be set once the antennas are deployed")
        pass
