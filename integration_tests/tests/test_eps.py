from devices.eps import HousekeepingA, HousekeepingB
from obc import ResetWatchdogOn
from system import runlevel
from tests.base import RestartPerSuite
from utils import TestEvent


@runlevel(1)
class EPSTest(RestartPerSuite):
    def test_enable_lcl(self):
        ev = TestEvent()

        self.system.eps.TKmain.on_enable = ev.set

        self.system.obc.enable_lcl(0x01)

        self.assertTrue(ev.wait_for_change(1), "Should enable TKmain LCL (controller A)")

        ev = TestEvent()

        self.system.eps.TKred.on_enable = ev.set

        self.system.obc.enable_lcl(0x11)

        self.assertTrue(ev.wait_for_change(1), "Should enable TKred LCL (controller B)")

    def test_disable_lcl(self):
        ev = TestEvent()

        self.system.eps.TKmain.arrange_on()
        self.system.eps.TKmain.on_disable = ev.set

        self.system.obc.disable_lcl(0x01)

        self.assertTrue(ev.wait_for_change(1), "Should disable TKmain LCL (controller A)")

        ev = TestEvent()

        self.system.eps.TKred.arrange_on()
        self.system.eps.TKred.on_disable = ev.set

        self.system.obc.disable_lcl(0x11)

        self.assertTrue(ev.wait_for_change(1), "Should disable TKred LCL (controller B)")

    def test_disable_overheat_mode(self):
        ev = TestEvent()

        self.system.eps.controller_a.on_disable_overheat_submode = ev.set

        self.assertTrue(self.system.obc.disable_overheat_submode('A'))

        self.assertTrue(ev.wait_for_change(1), "Should disable overheat mode (controller A)")

        ev = TestEvent()

        self.system.eps.controller_b.on_disable_overheat_submode = ev.set

        self.assertTrue(self.system.obc.disable_overheat_submode('B'))

        self.assertTrue(ev.wait_for_change(1), "Should disable overheat mode (controller B)")

    def test_enable_burn_switch(self):
        ev = TestEvent()

        self.system.eps.SAILmain.on_enable = ev.set

        self.system.obc.enable_burn_switch(True, 1)

        self.assertTrue(ev.wait_for_change(1), "Burn switch should be enabled (controller A)")

        ev = TestEvent()

        self.system.eps.SAILred.on_enable = ev.set

        self.system.obc.enable_burn_switch(False, 1)

        self.assertTrue(ev.wait_for_change(1), "Burn switch should be enabled (controller B)")

    def test_read_housekeeping_a(self):
        expected = HousekeepingA()

        expected.MPPT_X.SOL_CURR = 1
        expected.MPPT_X.SOL_VOLT = 2
        expected.MPPT_X.SOL_OUT_VOLT = 3
        expected.MPPT_X.TEMP = 4
        expected.MPPT_X.STATE = 5

        expected.MPPT_Y_PLUS.SOL_CURR = 6
        expected.MPPT_Y_PLUS.SOL_VOLT = 7
        expected.MPPT_Y_PLUS.SOL_OUT_VOLT = 8
        expected.MPPT_Y_PLUS.TEMP = 9
        expected.MPPT_Y_PLUS.STATE = 10

        expected.MPPT_Y_MINUS.SOL_CURR = 11
        expected.MPPT_Y_MINUS.SOL_VOLT = 12
        expected.MPPT_Y_MINUS.SOL_OUT_VOLT = 13
        expected.MPPT_Y_MINUS.TEMP = 14
        expected.MPPT_Y_MINUS.STATE = 15

        expected.DISTR.CURR_3V3 = 16
        expected.DISTR.VOLT_3V3 = 17
        expected.DISTR.CURR_5V = 18
        expected.DISTR.VOLT_5V = 19
        expected.DISTR.CURR_VBAT = 20
        expected.DISTR.VOLT_VBAT = 21
        expected.DISTR.LCL_STATE = 23
        expected.DISTR.LCL_FLAGB = 24

        expected.BATC.VOLT_A = 25
        expected.BATC.CHRG_CURR = 26
        expected.BATC.DCHRG_CURR = 27
        expected.BATC.TEMP = 28
        expected.BATC.STATE = 29

        expected.BP.TEMP_A = 30
        expected.BP.TEMP_B = 31

        expected.CTRLB.VOLT_3V3d = 32

        expected.CTRLA.SAFETY_CTR = 33
        expected.CTRLA.PWR_CYCLES = 34
        expected.CTRLA.UPTIME = 35
        expected.CTRLA.TEMP = 36
        expected.CTRLA.SUPP_TEMP = 39

        expected.DCDC3V3.TEMP = 37
        expected.DCDC5V.TEMP = 38

        self.system.eps.controller_a.on_get_housekeeping = lambda: expected

        result = self.system.obc.read_housekeeping_a()

        self.assertEqual(result, expected)

    def test_read_housekeeping_b(self):
        expected = HousekeepingB()
        expected.BP.TEMP_C = 1
        expected.BATC.VOLT_B = 2
        expected.CTRLA.VOLT_3V3d = 3
        expected.CTRLB.SAFETY_CTR = 4
        expected.CTRLB.PWR_CYCLES = 5
        expected.CTRLB.UPTIME = 6
        expected.CTRLB.TEMP = 7
        expected.CTRLB.SUPP_TEMP = 8

        self.system.eps.controller_b.on_get_housekeeping = lambda: expected

        result = self.system.obc.read_housekeeping_b()

        self.assertEqual(result, expected)

    def test_fail_to_enable_lcl_should_raise_error_count(self):
        self.system.i2c.enable_bus_devices([self.system.eps.controller_a.address], False)
        result = self.system.obc.enable_lcl(0x01)

        self.assertEqual(result, 0xFF + 1)

        counters = self.system.obc.error_counters()

        self.assertEqual(counters[self.system.eps.ERROR_COUNTER].current, 5)
        self.system.i2c.enable_bus_devices([self.system.eps.controller_a.address], True)

    def test_reset_watchdog_a(self):
        ev = TestEvent()
        self.system.eps.controller_a.on_reset_watchdog = ev.set

        self.system.obc.eps_reset_watchdog(ResetWatchdogOn.A)

        self.assertTrue(ev.wait_for_change(1), "Watchdog should be reseted (controller A)")

    def test_reset_watchdog_b(self):
        ev = TestEvent()
        self.system.eps.controller_b.on_reset_watchdog = ev.set

        self.system.obc.eps_reset_watchdog(ResetWatchdogOn.B)

        self.assertTrue(ev.wait_for_change(1), "Watchdog should be reseted (controller A)")

    def test_reset_watchdog_both(self):
        ev_a = TestEvent()
        ev_b = TestEvent()

        self.system.eps.controller_a.on_reset_watchdog = ev_a.set
        self.system.eps.controller_b.on_reset_watchdog = ev_b.set

        self.system.obc.eps_reset_watchdog(ResetWatchdogOn.Both)

        self.assertTrue(ev_a.wait_for_change(1), "Watchdog should be reseted (controller A)")
        self.assertTrue(ev_b.wait_for_change(1), "Watchdog should be reseted (controller B)")
