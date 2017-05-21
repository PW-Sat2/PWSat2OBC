import logging
from unittest import skip

from nose.tools import nottest

from tests.base import BaseTest


class WatchdogTest(BaseTest):
    @skip('Manual test')
    def test_should_restart_when_mcu_hangs(self):
        self.system.obc.hang()
        rebooted = self.system.obc.wait_for_boot(70)
        logging.info("flag=%s", str(rebooted))
        if not rebooted:
            self.system.restart()  # allow clean test tear down

        self.assertTrue(rebooted, "MCU should be rebooted")
