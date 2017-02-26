import logging

from system import wait_for_obc_start
from datetime import datetime, timedelta
from tests.base import BaseTest
import time

class TestTime(BaseTest):
    # @wait_for_obc_start()
    # def test_jump_to_time(self):
    #     wait_time_accuracy_ms = 200
    #     current_time_ms = self.system.obc.current_time()
    #     next_time_ms = current_time_ms + 1000 * 100
    #
    #     self.system.obc.jump_to_time(next_time_ms / 1000)
    #
    #     current_time_ms = self.system.obc.current_time()
    #
    #     self.assertAlmostEqual(current_time_ms, next_time_ms, places=None, delta=wait_time_accuracy_ms)
    #
    # @wait_for_obc_start()
    # def test_clock_running(self):
    #     wait_time_s = 3
    #     wait_time_accuracy_ms = 200
    #
    #     sys_start_time_s = time.time()
    #     obc_start_time_ms = self.system.obc.current_time()
    #
    #     time.sleep(wait_time_s)
    #
    #     sys_end_time_s = time.time()
    #     obc_end_time_ms = self.system.obc.current_time()
    #
    #     sys_time_difference_ms = 1000*(sys_end_time_s - sys_start_time_s)
    #     obc_time_difference_ms = obc_end_time_ms - obc_start_time_ms
    #
    #     self.assertAlmostEqual(obc_time_difference_ms, sys_time_difference_ms, places=None, delta=wait_time_accuracy_ms)

    @wait_for_obc_start()
    def test_time_correction(self):
        log = logging.getLogger("test_time_correction")

        start_time = datetime.now()
        obc_start_time = self.system.obc.current_time()

        log.info("Force first run of time correction")
        self.system.rtc.set_response_time(start_time)
        self.system.obc.run_mission()

        log.info("Move forward by enough time to cause another time correction")
        elapsed_time = timedelta(minutes=20)
        elapsed_time_error = timedelta(minutes=5)

        self.system.obc.jump_to_time(elapsed_time.total_seconds())
        self.system.rtc.set_response_time(start_time + elapsed_time + elapsed_time_error)

        log.info("Run another time correction")
        self.system.obc.run_mission()


        log.info("Get time")
        self.system.obc.current_time()