from system import wait_for_obc_start
from tests.base import BaseTest
import time

class TestTime(BaseTest):
    @wait_for_obc_start()
    def test_jump_to_time(self):
        current_time = self.system.obc.current_time()

        self.assertEqual(current_time, 0)

        self.system.obc.jump_to_time(100)

        current_time = self.system.obc.current_time()

        self.assertGreaterEqual(current_time, 100)

    @wait_for_obc_start()
    def test_clock_running(self):
        wait_time_s = 3
        wait_time_accuracy_s = 1

        start_time = self.system.obc.current_time()

        time.sleep(wait_time_s)

        end_time = self.system.obc.current_time()

        time_difference = end_time - start_time

        self.assertGreaterEqual(time_difference, wait_time_s - wait_time_accuracy_s)
        self.assertLessEqual(time_difference, wait_time_s + wait_time_accuracy_s)