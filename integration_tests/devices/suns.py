import logging
import struct

import i2cMock


def from_uint8(tab):
    return struct.unpack('B', ensure_string(tab[0:1]))[0]


def to_uint8(value):
    return list(struct.pack('B', value))


def to_uint16(value):
    return list(struct.pack('H', value))


def to_uint16_xyzw(tab):
    return to_uint16(tab[0]) + to_uint16(tab[1]) + to_uint16(tab[2]) + to_uint16(tab[3])


class SunS(i2cMock.I2CDevice):

    def __init__(self, gpio_driver, pin):
        super(SunS, self).__init__(0x44, "Imtq")

        self.interrupt_pin = pin
        self.gpioDriver = gpio_driver

        self.ack = 0
        self.presence = 1
        self.adc_valid = 2

        self.visible_light = [
            [ 3, 4, 5, 6 ],
            [ 7, 8, 9, 10 ],
            [ 11, 12, 13, 14 ]
        ]

        self.temperature_structure = 15
        self.temperature_panels = [ 16, 17, 18, 19 ]

        self.gain = 20
        self.itime = 21
        
        self.infrared = [
            [ 22, 23, 24, 25 ],
            [ 26, 27, 28, 29 ],
            [ 30, 31, 32, 33 ]
        ]

    def gpio_interrupt_low(self):
        self.gpioDriver.gpio_low(self.interrupt_pin)

    def gpio_interrupt_high(self):
        self.gpioDriver.gpio_high(self.interrupt_pin)

    # --- Commands ---

    @i2cMock.command([0x80])
    def _measure_suns(self, *data):
        self.log.info("Measure SunS")
        return []

    @i2cMock.command([0x0])
    def _read_registers(self, *data):
        return [0x11] + \
               to_uint16(self.ack) + \
               to_uint16(self.presence) + \
               to_uint16(self.adc_valid) + \
               to_uint16_xyzw(self.visible_light[0]) + \
               to_uint16_xyzw(self.visible_light[1]) + \
               to_uint16_xyzw(self.visible_light[2]) + \
               to_uint16(self.temperature_structure) + \
               to_uint16_xyzw(self.temperature_panels) + \
               to_uint8(self.gain) + \
               to_uint8(self.itime) + \
               to_uint16_xyzw(self.infrared[0]) + \
               to_uint16_xyzw(self.infrared[1]) + \
               to_uint16_xyzw(self.infrared[2])
