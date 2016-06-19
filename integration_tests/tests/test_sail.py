from base import BaseTest

INFINITY_TIME = 999999

class Test_SailTest(BaseTest):
    def test_pingpong(self):
        l = self.system.obc.ping()

        self.assertEqual("pong", l)

    def test_jump_to_time(self):
        self.system.obc.jump_to_time(100)

        current_time = self.system.obc.current_time()

        self.assertGreaterEqual(current_time, 100)

    def test_happy_path(self):
        self.system.obc.jump_to_time(INFINITY_TIME)

        self.system.eps.wait_for_sail_open()


