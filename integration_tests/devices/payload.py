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
        self.RadFET = [0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF]
        self.default_processing_time = 1

    def mock_processing_start(self):
        self.gpio_high(self.pin)
        self.log.info("Start PLD measurement")
        t = Timer(self.default_processing_time, self.mock_processing_stop)
        t.start()

        # reset default processing time
        self.default_processing_time = 1

    def mock_processing_stop(self):
        self.log.info("Finished PLD measurement")
        self.gpio_low(self.pin)

    def gpio_low(self, pin):
        self.gpioDriver.gpio_low(pin)

    def gpio_high(self, pin):
        self.gpioDriver.gpio_high(pin)

    # Commands

    @i2cMock.command([0x80])
    def measure_suns(self):
        self.log.debug("Measure SunS")
        self.SunS_Ref = [0x0101, 0x0102, 0x0103, 0x0104, 0x0105]
        self.mock_processing_start()

    @i2cMock.command([0x81])
    def measure_temperatures(self):
        self.log.debug("Measure Temperatures")
        self.Temperatures = [0x0201, 0x0202, 0x0203, 0x0204, 0x0205, 0x0206, 0x0207, 0x0208, 0x0209]
        self.mock_processing_start()

    @i2cMock.command([0x82])
    def measure_photodiodes(self):
        self.log.debug("Measure Photodiodes")
        self.Photodiodes = [0x0301, 0x0302, 0x0303, 0x0304, 0x0305]
        self.mock_processing_start()

    @i2cMock.command([0x83])
    def measure_housekeeping(self):
        self.log.debug("Measure Housekeeping")
        self.Housekeeping = [0x0401, 0x0402]
        self.mock_processing_start()

    @i2cMock.command([0x84])
    def measure_radfet(self):
        self.log.debug("Measure RadFET")
        self.RadFET = [0x00500001, 0x00500002, 0x00500003, 0x00500004]
        self.default_processing_time = 30
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
        return list(struct.pack('<LLLL',
                                self.RadFET[0],
                                self.RadFET[1],
                                self.RadFET[2],
                                self.RadFET[3]))
