import logging

import i2cMock
from i2cMock import I2CDevice


class RTCDevice(I2CDevice):
    def __init__(self):
        super(RTCDevice, self).__init__(0b1010001, "RTC")

        self.time_response = [0, 0, 0, 0, 0, 0, 0]

    def set_response_array(self, response_array):
        self.time_response = response_array

    def set_response_time(self, response_time):
        time = response_time.time()
        date = response_time.date()
        self.time_response = [
            self.to_bcd(time.second),
            self.to_bcd(time.minute),
            self.to_bcd(time.hour),
            self.to_bcd(date.day), 0,
            self.to_bcd(date.month),
            self.to_bcd(date.year - 2000)]

    def to_bcd(self, byte):
        return ((byte / 10) << 4) | (byte % 10)

    @i2cMock.command([0x02])
    def read_time(self):
        self.log.debug("Read Time")
        return self.time_response or [0, 0, 0, 0, 0, 0, 0]
