from datetime import timedelta

from base import BaseTest
from system import runlevel
from utils import TestEvent


class Test_SailTest(BaseTest):
    @runlevel(1)
    def test_pingpong(self):
        l = self.system.obc.ping()

        self.assertEqual("pong", l)

        boot_params = self.system.obc._command("boot_params")

        p = boot_params.find("runlevel=1")

        self.assertGreater(p, 0)

    @runlevel(2)
    def test_happy_path(self):
        self.system.obc.jump_to_time(timedelta(days=42))

        lcl_enabled = TestEvent()
        switch_enabled = TestEvent()

        self.system.eps.TKmain.on_enable = lcl_enabled.set
        self.system.eps.SAILmain.on_enable = switch_enabled.set

        self.assertTrue(lcl_enabled.wait_for_change(16))
        self.assertTrue(switch_enabled.wait_for_change(10))
