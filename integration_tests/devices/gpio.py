import logging

class GPIODriver(object):
    def __init__(self, i2cMocker):

        self.log = logging.getLogger("GPIO")
        self.i2cMocker = i2cMocker

    def gpio_low(self, pin):
        self.log.debug("Pin %d Low", pin)
        self.i2cMocker.gpio_low(pin)

    def gpio_high(self, pin):
        self.log.debug("Pin %d High", pin)
        self.i2cMocker.gpio_high(pin)
