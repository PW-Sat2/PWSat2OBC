from base import BaseTest
from system import wait_for_obc_start

INFINITY_TIME = 999999


class Test_SailTest(BaseTest):
    def test_pingpong(self):
        self.system.obc.wait_to_start();
        l = self.system.obc.ping()

        self.assertEqual("pong", l)

    @wait_for_obc_start()
    def test_happy_path(self):
        self.system.obc.jump_to_time(INFINITY_TIME)

        self.assertTrue(self.system.eps.sail0.wait_for_change(10))
        self.assertFalse(self.system.eps.sail0.wait_for_change(10))

        self.assertTrue(self.system.eps.sail1.wait_for_change(10))
        self.assertFalse(self.system.eps.sail1.wait_for_change(10))


