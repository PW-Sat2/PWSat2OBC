from math import ceil
from unittest import skip

from system import runlevel
from tests.base import RestartPerTest
from utils import TestEvent


class FDIRTest(RestartPerTest):
    @runlevel(2)
    @skip('Manual test')
    def test_should_trigger_power_cycle_after_reaching_error_limit(self):
        self.system.i2c.enable_bus_devices([self.system.comm.receiver.address], False)

        power_cycle_requested = TestEvent()

        self.system.eps.controller_a.on_power_cycle = power_cycle_requested.set

        limit = 128
        increment = 5
        errors = ceil(limit/increment)
        requested = power_cycle_requested.wait_for_change(errors * 1.5)

        self.assertTrue(requested, "Power cycle should be requested")