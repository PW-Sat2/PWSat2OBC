import devices
from tests.base import BaseTest
from obc import *
from utils import TestEvent
from system import auto_power_on, runlevel


class Test_Beacon(BaseTest):
    @auto_power_on(False)
    def __init__(self, methodName='runTest'):
        super(Test_Beacon, self).__init__(methodName)

    def begin_deployment(self):
        self.system.obc.jump_to_time(43 * 60)

    def next_step(self):
        self.system.obc.run_mission()

    def run_steps(self, count):
        while count > 0:
            self.next_step()
            count -= 1

    def begin(self, count=1):
        self.power_on_and_wait()
        self.begin_deployment()
        self.run_steps(count)

    @runlevel(1)
    def test_beacon_auto_activation(self):
        event = TestEvent()

        def reset_handler(*args):
            return False

        self.system.primary_antenna.begin_deployment()
        self.system.primary_antenna.finish_deployment()
        self.system.backup_antenna.begin_deployment()
        self.system.backup_antenna.finish_deployment()
        self.system.primary_antenna.on_reset = reset_handler
        self.system.backup_antenna.on_reset = reset_handler
        self.system.comm.transmitter.on_set_beacon = event.set
        self.begin(18)
        self.assertTrue(event.wait_for_change(1), "beacon should be set once the antennas are deployed")
