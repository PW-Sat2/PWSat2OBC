from base import BaseTest
from system import wait_for_obc_start
from utils import TestEvent

INFINITY_TIME = 999999


class Test_SailTest(BaseTest):
    def test_pingpong(self):
        self.system.obc.wait_to_start()
        l = self.system.obc.ping()

        self.assertEqual("pong", l)

    @wait_for_obc_start()
    def test_happy_path(self):
        self.system.obc.jump_to_time(INFINITY_TIME)

        lcl_enabled = TestEvent()
        switch_enabled = TestEvent()

        self.system.eps.TKmain.on_enable = lcl_enabled.set
        self.system.eps.SAILmain.on_enable = switch_enabled.set

        self.assertTrue(lcl_enabled.wait_for_change(16))
        self.assertTrue(switch_enabled.wait_for_change(10))
