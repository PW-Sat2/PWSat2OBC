import logging
import struct

import i2cMock
import time
from Queue import Queue, Empty
from threading import Lock
from utils import *
from build_config import config


def two_bytes(val):
    return val[0] + (val[1] << 8)


def to_int32(value):
    return [value & 0xFF,
            (value & 0xFF00) >> 8,
            (value & 0xFF0000) >> 16,
            (value & 0xFF000000) >> 24]


def to_int16(value):
    return [value & 0xFF,
            (value & 0xFF00) >> 8]


def to_int16_xyz(tab):
    return to_int16(tab[0]) + to_int16(tab[1]) + to_int16(tab[2])


def to_int32_xyz(tab):
    return to_int32(tab[0]) + to_int32(tab[1]) + to_int32(tab[2])


class Imtq(i2cMock.I2CDevice):
    def __init__(self):
        super(Imtq, self).__init__(0x10)
        self.log = logging.getLogger("Imtq")

        self.status = 0
        self.mode = 0
        self.error = 0
        self.conf = 0
        self.uptime = 0
        self.mtm_measurement = [0, 0, 0]
        self.coil_actuation = 0
        self.coil_current = [0, 0, 0]
        self.coil_temperature = [0, 0, 0]
        self.commanded_dipole = [0, 0, 0]
        self.bdot_data = [0, 0, 0]

        self.digital_voltage_raw = 0
        self.analog_voltage_raw = 0
        self.digital_current_raw = 0
        self.analog_current_raw = 0
        self.coil_current_raw = [0, 0, 0]
        self.coil_temperature_raw = [0, 0, 0]
        self.mcu_temperature_raw = 0

        self.digital_voltage = 0
        self.analog_voltage = 0
        self.digital_current = 0
        self.analog_current = 0
        self.mcu_temperature = 0

    # --- Commands ---

    @i2cMock.command([0xAA])
    def _software_reset(self, *data):
        if len(data) == 1 and data[0] == 0xA5:
            self.log.info("Software reset")
        else:
            self.log.info("Incorrect software reset command")

    @i2cMock.command([0x02])
    def _no_operation(self):
        self.log.info("No operation")
        return [0x02, self.status]

    @i2cMock.command([0x03])
    def _cancel_operation(self):
        self.log.info("Cancel operation")
        return [0x03, self.status]

    @i2cMock.command([0x04])
    def _start_mtm_measurement(self):
        self.log.info("Start MTM measurement")
        return [0x04, self.status]

    @i2cMock.command([0x05])
    def _start_actuation_current(self, *data):
        self.log.info("Start actuation (current)")
        return [0x05, self.status]

    @i2cMock.command([0x06])
    def _start_actuation_dipole(self, *data):
        self.log.info("Start actuation (current)")
        return [0x06, self.status]

    @i2cMock.command([0x08])
    def _start_self_test(self, *data):
        self.log.info("Start self-test with param %d", data[0])
        return [0x08, self.status]

    @i2cMock.command([0x09])
    def _start_bdot(self, *data):
        time = two_bytes(data[0:])
        self.log.info("Start BDot for %d", time)
        return [0x09, self.status]

    # --- Data retrieval ---

    @i2cMock.command([0x41])
    def _get_state(self):
        self.log.info("Get system state")
        return [0x41,
                self.status,
                self.mode,
                self.error,
                self.conf,
                self.uptime]

    @i2cMock.command([0x43])
    def _get_calibrated_mtm(self):
        self.log.info("Get calibrated MTM")
        return [0x43] + to_int32_xyz(self.mtm_measurement) + [self.coil_actuation]

    @i2cMock.command([0x44])
    def _get_coil_current(self):
        self.log.info("Get coil current")
        return [0x44] + to_int16_xyz(self.coil_current)

    @i2cMock.command([0x45])
    def _get_coil_temperature(self):
        self.log.info("Get coil temperature")
        return [0x45] + to_int16_xyz(self.coil_temperature)

    @i2cMock.command([0x46])
    def _get_dipole(self):
        self.log.info("Get commanded dipole")
        return [0x46] + to_int16_xyz(self.commanded_dipole)

    @i2cMock.command([0x47])
    def _get_self_test_result(self):
        self.log.info("Get self test result")
        stepval = [0x47, self.status, self.error] + 37 * [0]
        return 320*[0] #8 * stepval

    @i2cMock.command([0x48])
    def _get_detumble_data(self):
        self.log.info("Get detumble data")
        return [0x48, self.status] + \
               to_int32_xyz(self.mtm_measurement) + \
               to_int32_xyz(self.mtm_measurement) + \
               to_int32_xyz(self.bdot_data) + \
               to_int16_xyz(self.commanded_dipole) + \
               to_int16_xyz(self.coil_current) + \
               to_int16_xyz(self.coil_current)

    @i2cMock.command([0x49])
    def _get_detumble_data(self):
        self.log.info("Get RAW HK")
        return [0x49, self.status] + \
               to_int16(self.digital_voltage_raw) + \
               to_int16(self.analog_voltage_raw) + \
               to_int16(self.digital_current_raw) + \
               to_int16(self.analog_current_raw) + \
               to_int16_xyz(self.coil_current_raw) + \
               to_int16_xyz(self.coil_temperature_raw) + \
               to_int16(self.mcu_temperature_raw)

    @i2cMock.command([0x4A])
    def _get_detumble_data(self):
        self.log.info("Get Engineering HK")
        return [0x4A, self.status] + \
               to_int16(self.digital_voltage) + \
               to_int16(self.analog_voltage) + \
               to_int16(self.digital_current) + \
               to_int16(self.analog_current) + \
               to_int16_xyz(self.coil_current) + \
               to_int16_xyz(self.coil_temperature) + \
               to_int16(self.mcu_temperature)