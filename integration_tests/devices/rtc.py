import datetime
import logging
from threading import Lock

import i2cMock
from i2cMock import I2CDevice
from utils import RepeatedTimer


class RTCDevice(I2CDevice):
    def __init__(self):
        super(RTCDevice, self).__init__(0b1010001, "RTC")

        self._current_time = datetime.datetime.now()
        self._offset = datetime.timedelta()
        self._raw_response = None
        self._timer = RepeatedTimer(1, self._tick_time)
        self._lock = Lock()
        self._advance_time_interval = datetime.timedelta(seconds=1)

    def set_response_array(self, response_array):
        self._raw_response = response_array

    def set_response_time(self, response_time):
        with self._lock:
            self._current_time = response_time
        self._raw_response = None

    def response_time(self):
        with self._lock:
            return self._current_time + self._offset

    def start_running(self):
        self._timer.start()

    def stop_running(self):
        self._timer.stop()

    def advance_by(self, interval):
        with self._lock:
            self._offset += interval

    def _time_to_bcd(self, dt):
        time = dt.time()
        date = dt.date()
        return [
            self.to_bcd(time.second),
            self.to_bcd(time.minute),
            self.to_bcd(time.hour),
            self.to_bcd(date.day), 0,
            self.to_bcd(date.month),
            self.to_bcd(date.year - 2000)]

    def _tick_time(self):
        with self._lock:
            self._current_time += self._advance_time_interval

    def to_bcd(self, byte):
        return ((byte / 10) << 4) | (byte % 10)

    @i2cMock.command([0x02])
    def read_time(self):
        self.log.debug("Read Time")
        with self._lock:
            return self._raw_response or self._time_to_bcd(self._current_time + self._offset)


