import unittest

from system import System
from pins import Pins
import extensions

try:
    from config import config
except ImportError as e:
    raise ImportError(
        "Error loading config: %s. Did you forget to add <build>/integration_tests to PYTHONPATH?" % e.message)


class BaseTest(unittest.TestCase):
    def setUp(self):
        obc_com = config['OBC_COM']
        sys_bus_com = config['SYS_BUS_COM']
        payload_bus_com = config['PAYLOAD_BUS_COM']
        use_single_bus = config['SINGLE_BUS']
        gpio_com = config['GPIO_COM']

        self.gpio = Pins(gpio_com)

        extensions.set_up(test_id=self.id())

        self.system = System(obc_com, sys_bus_com, payload_bus_com, use_single_bus, self.gpio)

    def tearDown(self):
        self.system.close()
        self.gpio.close()

        extensions.tear_down(test_id=self.id())
