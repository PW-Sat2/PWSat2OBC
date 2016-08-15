from base import BaseTest

INFINITY_TIME = 999999


class Test_SailTest(BaseTest):
    def test_pingpong(self):
        self.system.obc.wait_to_start();
        l = self.system.obc.ping()

        self.assertEqual("pong", l)

    def test_jump_to_time(self):
        self.system.obc.wait_to_start();
        self.system.obc.jump_to_time(100)

        current_time = self.system.obc.current_time()

        self.assertGreaterEqual(current_time, 100)

    def test_happy_path(self):
        self.system.obc.wait_to_start();
        self.system.obc.jump_to_time(INFINITY_TIME)

        self.assertTrue(self.system.eps.sail0.wait_for_change(1))
        self.assertFalse(self.system.eps.sail0.wait_for_change(1))

        self.assertTrue(self.system.eps.sail1.wait_for_change(1))
        self.assertFalse(self.system.eps.sail1.wait_for_change(1))


