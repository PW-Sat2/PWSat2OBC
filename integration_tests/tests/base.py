import unittest

from system import System
from pins import Pins

try:
    from config import config
except ImportError as e:
    raise ImportError(
        "Error loading config: %s. Did you forget to add <build>/integration_tests to PYTHONPATH?" % e.message)


class BaseTest(unittest.TestCase):
    auto_power_on = True

    def setUp(self):
        obc_com = config['OBC_COM']
        sys_bus_com = config['SYS_BUS_COM']
        payload_bus_com = config['PAYLOAD_BUS_COM']
        use_single_bus = config['SINGLE_BUS']
        gpio_com = config['GPIO_COM']

        self.gpio = Pins(gpio_com)

        self.system = System(obc_com, sys_bus_com, payload_bus_com, use_single_bus, self.gpio, self.auto_power_on)

    def tearDown(self):
        self.system.close()
        self.gpio.close()

        #del self.gpio
