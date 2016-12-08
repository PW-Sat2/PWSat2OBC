import unittest

from system import System
from pins import Pins
import extensions
from build_config import config


class BaseTest(unittest.TestCase):
    def __init__(self, *args, **kwargs):
        super(BaseTest, self).__init__(*args, **kwargs)
        self.auto_power_on = True

    def setUp(self):
        obc_com = config['OBC_COM']
        sys_bus_com = config['SYS_BUS_COM']
        payload_bus_com = config['PAYLOAD_BUS_COM']
        use_single_bus = config['SINGLE_BUS']
        gpio_com = config['GPIO_COM']

        self.gpio = Pins(gpio_com)

        extensions.set_up(test_id=self.id())

        self.system = System(obc_com, sys_bus_com, payload_bus_com, use_single_bus, self.gpio, self.auto_power_on)

    def tearDown(self):
        self.system.close()
        self.gpio.close()
        extensions.tear_down(test_id=self.id())

    def power_on_obc(self):
        self.system.obc.power_on()

    def power_on_and_wait(self):
        self.power_on_obc()
        self.system.obc.wait_to_start()

