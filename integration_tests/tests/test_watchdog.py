import logging
from unittest import skip

from nose.tools import nottest

from system import wait_for_obc_start
from tests.base import BaseTest
from utils import TestEvent


class WatchdogTest(BaseTest):
    @skip('Manual test')
    def test_should_restart_when_mcu_hangs(self):
        self.system.obc.hang()
        rebooted = self.system.obc.wait_for_boot(70)
        logging.info("flag=%s", str(rebooted))
        if not rebooted:
            self.system.restart()  # allow clean test tear down

        self.assertTrue(rebooted, "MCU should be rebooted")

    @wait_for_obc_start()
    def test_should_kick_watchdog_in_mission_loop(self):
        self.system.obc.suspend_mission()

        ev_a = TestEvent()
        ev_b = TestEvent()

        self.system.eps.controller_a.on_reset_watchdog = ev_a.set
        self.system.eps.controller_b.on_reset_watchdog = ev_b.set

        self.system.obc.run_mission()

        self.assertTrue(ev_a.wait_for_change(1), "Watchdog should be kicked (A)")
        self.assertTrue(ev_b.wait_for_change(1), "Watchdog should be kicked (B)")