import logging
import struct

import i2cMock
from i2cMock import I2CDevice
from threading import Timer


class Payload(I2CDevice):
    def __init__(self, gpioDriver, pin):
        super(Payload, self).__init__(0b0110000)

        self.log = logging.getLogger("Payload")
        self.pin = pin
        self.gpioDriver = gpioDriver
        self.whoami = 0x53
        self.SunS_Ref = [0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF]
        self.Temperatures = [0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF]
        self.Photodiodes = [0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF]
        self.Housekeeping = [0xFFFF, 0xFFFF]
        self.RadFET = [0xFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF]
        self.default_processing_time = 1
        self.long_processing_time = 10

    def mock_processing_start(self, is_long_processing = False):
        self.gpio_high(self.pin)
        self.log.info("Start PLD measurement")

        timeout = self.long_processing_time if is_long_processing else self.default_processing_time
        t = Timer(timeout, self.mock_processing_stop)
        t.start()

    def mock_processing_stop(self):
        self.log.info("Finished PLD measurement")
        self.gpio_low(self.pin)

    def gpio_low(self, pin):
        self.gpioDriver.gpio_low(pin)

    def gpio_high(self, pin):
        self.gpioDriver.gpio_high(pin)

    # Value setters

    def set_suns_ref(self, new_value):
        self.SunS_Ref = new_value

    def set_temperatures(self, new_value):
        self.Temperatures = new_value

    def set_photodiodes(self, new_value):
        self.Photodiodes = new_value

    def set_housekeeping(self, new_value):
        self.Housekeeping = new_value

    def set_radfet(self, new_value):
        self.RadFET = new_value

    def set_timeout(self, new_value = 1):
        self.default_processing_time = new_value

    def set_long_timeout(self, new_value = 30):
        self.long_processing_time = new_value

    # Measurements mock

    def mock_measure_suns_ref(self):
        if self.SunS_Ref == [0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF]:
            self.SunS_Ref = [101, 102, 103, 104, 105]

    def mock_measure_temperatures(self):
        if self.Temperatures == [0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF]:
            self.set_temperatures([201, 202, 203, 204, 205, 206, 207, 208, 209])

    def mock_measure_photodiodes(self):
        if self.Photodiodes == [0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF]:
            self.Photodiodes = [301, 302, 303, 304]

    def mock_measure_housekeeping(self):
        if self.Housekeeping == [0xFFFF, 0xFFFF]:
            self.Housekeeping = [401, 402]

    def mock_initialize_radfet(self):
        self.RadFET = [50, 500001, 500002, 500003, 500004]

    def mock_measure_radfet(self):
        if self.RadFET != [0xFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF]:
            self.RadFET[0] = self.RadFET[0] + 1
            self.RadFET[1] = self.RadFET[1] + 100
            self.RadFET[2] = self.RadFET[2] + 100
            self.RadFET[3] = self.RadFET[3] + 100
            self.RadFET[4] = self.RadFET[4] + 100
        else:
            # mock read of unintialized radfet
            self.RadFET = [99, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF]

    def mock_finish_radfet(self):
        if self.RadFET != [0xFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF]:
            self.RadFET = [59, 590001, 590002, 590003, 590004]
        else:
            # mock read of unintialized radfet
            self.RadFET = [111, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF, 0x00FFFFFF]

    # Commands

    @i2cMock.command([0x80])
    def measure_suns(self):
        self.log.debug("Measure SunS")
        self.mock_measure_suns_ref()
        self.mock_processing_start()

    @i2cMock.command([0x81])
    def measure_temperatures(self):
        self.log.debug("Measure Temperatures")
        self.mock_measure_temperatures()
        self.mock_processing_start()

    @i2cMock.command([0x82])
    def measure_photodiodes(self):
        self.log.debug("Measure Photodiodes")
        self.mock_measure_photodiodes()
        self.mock_processing_start()

    @i2cMock.command([0x83])
    def measure_housekeeping(self):
        self.log.debug("Measure Housekeeping")
        self.mock_measure_housekeeping()
        self.mock_processing_start()

    @i2cMock.command([0x84])
    def radfet_on(self):
        self.log.debug("RadFET On")
        self.mock_initialize_radfet()
        self.mock_processing_start()

    @i2cMock.command([0x85])
    def measure_radfet(self):
        self.log.debug("Measure RadFET")
        self.mock_measure_radfet()
        self.mock_processing_start(True)

    @i2cMock.command([0x86])
    def radfet_off(self):
        self.log.debug("RadFET off")
        self.mock_finish_radfet()
        self.mock_processing_start()

    # Data Reads

    @i2cMock.command([0x00])
    def read_whoami(self):
        self.log.debug("Read Who am I")
        return list(struct.pack('<B', self.whoami))

    @i2cMock.command([0x01])
    def read_suns_ref(self):
        self.log.debug("Read SunS Voltages")
        return list(struct.pack('<HHHHH',
                                self.SunS_Ref[0],
                                self.SunS_Ref[1],
                                self.SunS_Ref[2],
                                self.SunS_Ref[3],
                                self.SunS_Ref[4]))

    @i2cMock.command([11])
    def read_temperatures(self):
        self.log.debug("Read Temperatures")
        return list(struct.pack('<HHHHHHHHH',
                                self.Temperatures[0],
                                self.Temperatures[1],
                                self.Temperatures[2],
                                self.Temperatures[3],
                                self.Temperatures[4],
                                self.Temperatures[5],
                                self.Temperatures[6],
                                self.Temperatures[7],
                                self.Temperatures[8]))

    @i2cMock.command([29])
    def read_photodiodes(self):
        self.log.debug("Read Photodiodes")
        return list(struct.pack('<HHHH',
                                self.Photodiodes[0],
                                self.Photodiodes[1],
                                self.Photodiodes[2],
                                self.Photodiodes[3]))

    @i2cMock.command([37])
    def read_housekeeping(self):
        self.log.debug("Read Housekeeping")
        return list(struct.pack('<HH', self.Housekeeping[0], self.Housekeeping[1]))

    @i2cMock.command([41])
    def read_radfet(self):
        self.log.debug("Read RadFET")
        return list(struct.pack('<BLLLL',
                                self.RadFET[0],
                                self.RadFET[1],
                                self.RadFET[2],
                                self.RadFET[3],
                                self.RadFET[4]))
