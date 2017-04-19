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
        mock_com = config['MOCK_COM']
        gpio_com = config['GPIO_COM']

        self.gpio = Pins(gpio_com)

        extensions.set_up(test_id=self.id())

        self.system = System(obc_com, mock_com, self.gpio, self.auto_power_on)

    def tearDown(self):
        self.system.obc.sync_fs()

        self.system.close()
        self.gpio.close()
        extensions.tear_down(test_id=self.id())

    def power_on_obc(self):
        self.system.obc.power_on()

    def power_on_and_wait(self):
        self.power_on_obc()
        self.system.obc.wait_to_start()

