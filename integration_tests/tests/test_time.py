import logging
import time
from datetime import datetime, timedelta

from nose.tools import nottest

from system import runlevel
from tests.base import BaseTest


class TestTime(BaseTest):
    def test_jump_to_time(self):
        wait_time_accuracy = timedelta(milliseconds=200)
        current_time = self.system.obc.current_time()
        next_time = current_time + timedelta(seconds=100)

        self.system.obc.jump_to_time(next_time)

        current_time = self.system.obc.current_time()

        self.assertAlmostEqual(current_time, next_time, places=None, delta=wait_time_accuracy)

    def test_clock_running(self):
        wait_time_s = 3
        wait_time_accuracy = timedelta(milliseconds=200)

        sys_start_time_s = time.time()
        obc_start_time = self.system.obc.current_time()

        time.sleep(wait_time_s)

        sys_end_time_s = time.time()
        obc_end_time = self.system.obc.current_time()

        sys_time_difference = timedelta(seconds=sys_end_time_s - sys_start_time_s)
        obc_time_difference = obc_end_time - obc_start_time

        self.assertAlmostEqual(obc_time_difference, sys_time_difference, places=None, delta=wait_time_accuracy)

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
        log = logging.getLogger("test_time_correction")

        self.system.rtc.set_response_time(start_time)
        self.system.obc.jump_to_time(0)
        obc_start = self.system.obc.current_time()

        log.info("Move forward by enough time to cause another time correction")
        elapsed_time_delta = timedelta(minutes=20)
        elapsed_time_error_delta = timedelta(minutes=5)

        self.system.obc.advance_time(elapsed_time_delta)
        self.system.rtc.set_response_time(start_time + elapsed_time_delta + elapsed_time_error_delta)

        log.info("Run another time correction")
        self.system.obc.run_mission()

        log.info("Get time")
        corrected = self.system.obc.current_time()
        expected = obc_start + elapsed_time_delta + (elapsed_time_error_delta / 2)

        self.assertEquals(expected, corrected)

    @runlevel(1)
    def test_time_correction_on_month_boundary(self):
        self.run_time_correction_test(datetime(year=2017, month=3, day=31, hour=23, minute=38, second=00))

    @runlevel(1)
    def test_time_correction_now(self):
        self.run_time_correction_test(datetime.now())
