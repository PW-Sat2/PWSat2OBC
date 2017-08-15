from threading import Timer

from enum import Enum

import i2cMock
from utils import *


def from_int16(tab):
    return struct.unpack('h', ensure_string(tab[0:2]))[0]


def from_uint16(tab):
    return struct.unpack('H', ensure_string(tab[0:2]))[0]


def from_int32(tab):
    return struct.unpack('i', ensure_string(tab[0:4]))[0]


def to_uint32(value):
    return list(struct.pack('I', value))


def to_int32(value):
    return list(struct.pack('i', value))


def to_uint16(value):
    return list(struct.pack('H', value))


def to_int16(value):
    return list(struct.pack('h', value))


def to_int16_xyz(tab):
    return to_int16(tab[0]) + to_int16(tab[1]) + to_int16(tab[2])


def to_uint16_xyz(tab):
    return to_uint16(tab[0]) + to_uint16(tab[1]) + to_uint16(tab[2])


def to_int32_xyz(tab):
    return to_int32(tab[0]) + to_int32(tab[1]) + to_int32(tab[2])


def to_uint32_xyz(tab):
    return to_uint32(tab[0]) + to_uint32(tab[1]) + to_uint32(tab[2])


class Imtq(i2cMock.I2CDevice):

    class Mode:
        class Type(Enum):
            idle = 0
            selftest = 1
            detumble = 2

        def __init__(self):
            self.mode = self.Type.idle

        def array(self):
            return [self.mode.value]

        @property
        def value(self):
            return self.mode.value

        def cancel_operation(self):
            self.mode = self.Type.idle

        def start_detumble(self, time_in_seconds):
            self.mode = self.Type.detumble
            self.timer = Timer(time_in_seconds, self.cancel_operation)
            self.timer.start()

        def start_selftest(self):
            self.mode = self.Type.selftest
            self.timer = Timer(0.5, self.cancel_operation)
            self.timer.start()

    def __init__(self):
        super(Imtq, self).__init__(0x10, "Imtq")

        self.status = 0
        self.mode = self.Mode()
        self.error = 0
        self.conf = 0
        self.uptime = 0
        self.mtm_measurement = [1, 2, 3]
        self.coil_actuation = 0
        self.coil_current = [4, 5, 6]
        self.coil_temperature = [7, 8, 9]
        self.commanded_dipole = [10, 11, 12]
        self.bdot_data = [13, 14, 15]

        self.digital_voltage_raw = 16
        self.analog_voltage_raw = 17
        self.digital_current_raw = 18
        self.analog_current_raw = 19
        self.coil_current_raw = [20, 21, 22]
        self.coil_temperature_raw = [23, 24, 25]
        self.mcu_temperature_raw = 26

        self.digital_voltage = 27
        self.analog_voltage = 28
        self.digital_current = 29
        self.analog_current = 30
        self.mcu_temperature = 31

        # Type: () -> [int,int,int]
        self.on_mtm_measurement = None

    def update_mtm(self, value):
        self.mtm_measurement = call(self.on_mtm_measurement, default=[value])
        self.status = (1 << 7)

    def update_actuation(self, value):
        self.commanded_dipole = value
        self.coil_current = value
        self.coil_current_raw = value
        self.coil_actuation = 1

    def clear_actuation(self):
        self.commanded_dipole = [0, 0, 0]
        self.coil_current = [0, 0, 0]
        self.coil_current_raw = [0, 0, 0]
        self.coil_actuation = 0

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
        self.mode.cancel_operation()
        self.clear_actuation()
        return [0x03, self.status]

    @i2cMock.command([0x04])
    def _start_mtm_measurement(self):
        self.log.info("Start MTM measurement")
        mtm_timer = Timer(0.01, self.update_mtm, [self.mtm_measurement])
        mtm_timer.start()
        return [0x04, self.status]

    @i2cMock.command([0x05])
    def _start_actuation_current(self, *data):
        current = [from_int16(data[i:i + 2]) for i in [0, 2, 4]]
        time = from_uint16(data[6:])
        self.log.info("Start actuation (current): " + str(current) + " for: " + str(time))
        self.update_actuation(current)
        t = Timer(time / 1000.0, self.clear_actuation)
        t.start()
        return [0x05, self.status]

    @i2cMock.command([0x06])
    def _start_actuation_dipole(self, *data):
        dipole = [from_int16(data[i:i+2]) for i in [0, 2, 4]]
        time = from_uint16(data[6:])
        self.update_actuation(dipole)
        t = Timer(time / 1000.0, self.clear_actuation)
        t.start()
        self.log.info("Start actuation (dipole): " + str(dipole) + " for: " + str(time))
        return [0x06, self.status]

    @i2cMock.command([0x08])
    def _start_self_test(self, *data):
        self.log.info("Start self-test with param %d", data[0])
        self.mode.start_selftest()
        return [0x08, self.status]

    @i2cMock.command([0x09])
    def _start_bdot(self, *data):
        time = from_uint16(data[0:])
        self.log.info("Start BDot for %d", time)
        self.mode.start_detumble(time)
        return [0x09, self.status]

    # --- Data retrieval ---

    @i2cMock.command([0x41])
    def _get_state(self):
        self.log.info("Get system state")
        return [0x41,
                self.status,
                self.mode.value,
                self.error,
                self.conf,
                self.uptime]

    @i2cMock.command([0x43])
    def _get_calibrated_mtm(self):
        self.log.info("Get calibrated MTM")
        resp = [0x43, self.status] + to_int32_xyz(self.mtm_measurement) + [self.coil_actuation]
        self.status = 0
        return resp

    @i2cMock.command([0x44])
    def _get_coil_current(self):
        self.log.info("Get coil current")
        return [0x44, self.status] + to_int16_xyz(self.coil_current)

    @i2cMock.command([0x45])
    def _get_coil_temperature(self):
        self.log.info("Get coil temperature")
        return [0x45, self.status] + to_int16_xyz(self.coil_temperature)

    @i2cMock.command([0x46])
    def _get_dipole(self):
        self.log.info("Get commanded dipole")
        return [0x46, self.status] + to_int16_xyz(self.commanded_dipole)

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
    def _get_raw_housekeeping_data(self):
        self.log.info("Get RAW HK")
        return [0x49, self.status] + \
               to_uint16(self.digital_voltage_raw) + \
               to_uint16(self.analog_voltage_raw) + \
               to_uint16(self.digital_current_raw) + \
               to_uint16(self.analog_current_raw) + \
               to_uint16_xyz(self.coil_current_raw) + \
               to_uint16_xyz(self.coil_temperature_raw) + \
               to_uint16(self.mcu_temperature_raw)

    @i2cMock.command([0x4A])
    def _get_eng_housekeeping_data(self):
        self.log.info("Get Engineering HK")
        return [0x4A, self.status] + \
               to_uint16(self.digital_voltage) + \
               to_uint16(self.analog_voltage) + \
               to_uint16(self.digital_current) + \
               to_uint16(self.analog_current) + \
               to_int16_xyz(self.coil_current) + \
               to_int16_xyz(self.coil_temperature) + \
               to_int16(self.mcu_temperature)