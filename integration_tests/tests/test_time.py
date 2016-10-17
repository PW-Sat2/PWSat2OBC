from system import wait_for_obc_start
from tests.base import BaseTest


class TestTime(BaseTest):
    @wait_for_obc_start()
    def test_jump_to_time(self):
        current_time = self.system.obc.current_time()

        self.assertEqual(current_time, 0)

        self.system.obc.jump_to_time(100)

        current_time = self.system.obc.current_time()

        self.assertGreaterEqual(current_time, 100)

