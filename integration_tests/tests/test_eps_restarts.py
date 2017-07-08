from obc import PowerCycleBy
from system import runlevel
from tests.base import RestartPerTest
from utils import TestEvent


class EPSRestartTest(RestartPerTest):
    @runlevel(1)
    def test_power_cycle_a(self):
        ev = TestEvent()

        self.system.eps.controller_a.on_power_cycle = ev.set

        self.system.obc.power_cycle(PowerCycleBy.A)

        self.assertTrue(ev.wait_for_change(1), "Should trigger power cycle (controller A)")
        self.system.restart()

    @runlevel(1)
    def test_power_cycle_b(self):
        ev = TestEvent()

        self.system.eps.controller_b.on_power_cycle = ev.set

        self.system.obc.power_cycle(PowerCycleBy.B)

        self.assertTrue(ev.wait_for_change(1), "Should trigger power cycle (controller A)")
        self.system.restart()

    @runlevel(1)
    def test_power_cycle_auto_fallback_to_b_when_not_restart_by_a(self):
        pc_a = TestEvent()
        pc_b = TestEvent()

        self.system.eps.controller_a.on_power_cycle = pc_a.set
        self.system.eps.controller_b.on_power_cycle = pc_b.set

        self.system.obc.power_cycle(PowerCycleBy.Auto)

        self.assertTrue(pc_a.wait_for_change(1), "Should trigger power cycle (controller A)")
        self.assertTrue(pc_b.wait_for_change(10), "Should trigger power cycle (controller B)")

        self.system.restart()

    @runlevel(1)
    def test_power_cycle_auto_fallback_to_b_when_a_not_responding(self):
        pc_b = TestEvent()

        self.system.i2c.enable_bus_devices([self.system.eps.controller_a.address], False)

        self.system.eps.controller_b.on_power_cycle = pc_b.set

        self.system.obc.power_cycle(PowerCycleBy.Auto)

        self.assertTrue(pc_b.wait_for_change(10), "Should trigger power cycle (controller B)")

        self.system.restart()
