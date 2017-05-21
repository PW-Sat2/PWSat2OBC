import logging
import unittest

from system import System
from pins import Pins
import extensions
from build_config import config
from obc.boot import BootToIndex, BootToUpper, BootHandlerChain


class BaseTest(unittest.TestCase):
    def __init__(self, *args, **kwargs):
        super(BaseTest, self).__init__(*args, **kwargs)
        self.auto_power_on = True

    def setUp(self):
        log = logging.getLogger("BaseTest")
        log.info("Starting test setup")

        obc_com = config['OBC_COM']
        mock_com = config['MOCK_COM']
        gpio_com = config['GPIO_COM']
        boot_handler = BootToUpper() if config['BOOT_UPPER'] else BootToIndex(config['BOOT_INDEX'])

        self.gpio = Pins(gpio_com)

        extensions.set_up(test_id=self.id())

        boot_wrappers = self._get_boot_wrappers(self._testMethodName)

        boot_wrappers += [boot_handler]

        boot_handler_chain = BootHandlerChain(boot_wrappers)

        self.system = System(obc_com, mock_com, self.gpio, boot_handler_chain, self.auto_power_on)
        if self.auto_power_on:
            self.system.obc.wait_to_start()

        log.info("Test setup finished")

    def tearDown(self):
        log = logging.getLogger("BaseTest")
        log.info("Starting test tear down")
        self.system.obc.sync_fs()

        self.system.close()
        self.gpio.close()
        extensions.tear_down(test_id=self.id())

        log.info("Test tear down finished")

    def power_on_obc(self, clean_state=False):
        self.system.obc.power_on(clean_state=clean_state)
        self.system.obc.wait_to_start()

    def _get_boot_wrappers(self, test):
        try:
            return self.__getattribute__(self._testMethodName).boot_wrappers or []
        except AttributeError:
            return []
