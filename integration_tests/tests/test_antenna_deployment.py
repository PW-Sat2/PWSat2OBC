import devices
from tests.base import BaseTest
from obc import *
from utils import TestEvent
from system import auto_power_on

class Test_AntennaDeployment(BaseTest):
    @auto_power_on(False)
    def __init__(self, methodName = 'runTest'):
        return super(Test_AntennaDeployment, self).__init__(methodName)

    def begin_deployment(self):
        self.system.obc.jump_to_time(42 * 60)

    def next_step(self, event):
        event.reset()
        self.system.obc.advance_time(10200)

    def test_antennas_are_not_deployed_too_soon(self):
        event = TestEvent()
        self.system.primary_antenna.on_begin_deployment = event.set
        self.power_on_and_wait()
        self.system.obc.jump_to_time(20 * 60)
        self.assertFalse(event.wait_for_change(1), "antenna deployment process began too soon")

    def test_process_begins_automatically(self):
        event = TestEvent()
        self.system.primary_antenna.on_reset = event.set
        self.power_on_and_wait()
        self.begin_deployment()
        self.assertTrue(event.wait_for_change(1), "antenna controller was not reset")
        self.system.primary_antenna.on_begin_deployment = event.set
        self.next_step(event)
        self.assertTrue(event.wait_for_change(1), "antenna deployment process did not began when it should")

    def test_deployment_is_cancelled_on_retry(self):
        event = TestEvent()
        self.system.primary_antenna.on_reset = event.set
        self.power_on_and_wait()
        self.begin_deployment()
        self.assertTrue(event.wait_for_change(1), "antenna controller was not reset")
        self.system.primary_antenna.on_deployment_cancel = event.set
        self.next_step(event)
        self.assertTrue(event.wait_for_change(1), "antenna deployment process was not cancelled")

    def test_deployment_completion(self):
        event = TestEvent()
        self.system.primary_antenna.on_reset = event.set
        self.power_on_and_wait()
        self.begin_deployment()
        self.assertTrue(event.wait_for_change(1), "antenna controller was not reset")
        self.system.primary_antenna.finish_deployment()
        self.system.primary_antenna.on_deployment_cancel = event.set
        self.next_step(event)
        self.assertTrue(event.wait_for_change(1), "antenna deployment on primary controller was not cancelled")
        self.system.backup_antenna.on_deployment_cancel = event.set
        self.next_step(event)
        self.assertTrue(event.wait_for_change(1), "antenna deployment on backup controller was not cancelled")

