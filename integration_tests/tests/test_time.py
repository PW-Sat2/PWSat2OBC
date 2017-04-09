import logging

from nose.tools import nottest
from nose_parameterized import parameterized

from system import wait_for_obc_start
from datetime import datetime, timedelta
from tests.base import BaseTest
import time


class TestTime(BaseTest):
    @wait_for_obc_start()
    def test_jump_to_time(self):
        wait_time_accuracy_ms = 200
        current_time_ms = self.system.obc.current_time()
        next_time_ms = current_time_ms + 1000 * 100

        self.system.obc.jump_to_time(next_time_ms / 1000)

        current_time_ms = self.system.obc.current_time()

        self.assertAlmostEqual(current_time_ms, next_time_ms, places=None, delta=wait_time_accuracy_ms)

    @wait_for_obc_start()
    def test_clock_running(self):
        wait_time_s = 3
        wait_time_accuracy_ms = 200

        sys_start_time_s = time.time()
        obc_start_time_ms = self.system.obc.current_time()

        time.sleep(wait_time_s)

        sys_end_time_s = time.time()
        obc_end_time_ms = self.system.obc.current_time()

        sys_time_difference_ms = 1000 * (sys_end_time_s - sys_start_time_s)
        obc_time_difference_ms = obc_end_time_ms - obc_start_time_ms

        self.assertAlmostEqual(obc_time_difference_ms, sys_time_difference_ms, places=None, delta=wait_time_accuracy_ms)

    @wait_for_obc_start()
    def test_rtc_communication(self):
        start_time = datetime.now()
        self.system.rtc.set_response_time(start_time)
        time_start = self.system.obc.rtc_duration()

        elapsed_time_delta = timedelta(minutes=5)
        self.system.rtc.set_response_time(start_time + elapsed_time_delta)
        time_end = self.system.obc.rtc_duration()

        self.assertEquals(5 * 60, time_end - time_start)

    @nottest
    def run_time_correction_test(self, start_time):
        self.system.obc.suspend_mission()
        log = logging.getLogger("test_time_correction")

        self.system.rtc.set_response_time(start_time)
        self.system.obc.jump_to_time(0)
        obc_start_s = self.system.obc.current_time() / 1000

        log.info("Move forward by enough time to cause another time correction")
        elapsed_time_delta = timedelta(minutes=20)
        elapsed_time_error_delta = timedelta(minutes=5)

        self.system.obc.advance_time(elapsed_time_delta.total_seconds() * 1000)
        self.system.rtc.set_response_time(start_time + elapsed_time_delta + elapsed_time_error_delta)

        log.info("Run another time correction")
        self.system.obc.run_mission()

        log.info("Get time")
        corrected_s = self.system.obc.current_time() / 1000
        expected_s = obc_start_s + elapsed_time_delta.total_seconds() + (elapsed_time_error_delta.total_seconds() / 2)

        self.assertEquals(expected_s, corrected_s)

    @wait_for_obc_start()
    def test_time_correction_on_month_boundary(self):
        self.run_time_correction_test(datetime(year=2017, month=3, day=31, hour=23, minute=38, second=00))

    @wait_for_obc_start()
    def test_time_correction_now(self):
        self.run_time_correction_test(datetime.now())
