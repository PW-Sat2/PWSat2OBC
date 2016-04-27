import os
import threading

from base import BaseTest

mock_com = os.environ.get('MOCK_COM')
obc_com = os.environ.get('OBC_COM')

INFINITY_TIME = 999999

class Test_SailTest(BaseTest):
    def test_pingpong(self):
        l = self.obc.ping()

        self.assertEqual("pong", l)

    def test_jump_to_time(self):
        self.obc.jump_to_time(INFINITY_TIME)

        current_time = self.obc.current_time()

        self.assertGreaterEqual(current_time, INFINITY_TIME)

    def test_happy_path(self):
        self.obc.jump_to_time(INFINITY_TIME)

        self.eps.wait_for_sail_open()


