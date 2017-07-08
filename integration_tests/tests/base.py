import logging
import unittest

import extensions
from build_config import config
from system import System


class BaseTest(unittest.TestCase):
    def __init__(self, *args, **kwargs):
        super(BaseTest, self).__init__(*args, **kwargs)
        self.auto_power_on = True

    def power_on_obc(self):
        self.system.restart(self.__class__._get_class_boot_wrappers() + self._get_boot_wrappers(self._testMethodName))

    def _get_boot_wrappers(self, test):
        try:
            return self.__getattribute__(self._testMethodName).boot_wrappers or []
        except AttributeError:
            return []

    @classmethod
    def _get_class_boot_wrappers(cls):
        try:
            return cls.boot_wrappers or []
        except AttributeError:
            return []


class RestartPerTest(BaseTest):
    def setUp(self):
        log = logging.getLogger("BaseTest")
        log.info("Starting test setup")

        extensions.set_up(test_id=self.id())

        (self.gpio, self.system) = System.build_from_config(config)

        boot_wrappers = self._get_boot_wrappers(self._testMethodName)

        if self.auto_power_on:
            self.system.restart(boot_wrappers)

        log.info("Test setup finished")

    def tearDown(self):
        log = logging.getLogger("BaseTest")
        log.info("Starting test tear down")
        self.system.obc.sync_fs()

        self.system.close()
        self.gpio.close()
        extensions.tear_down(test_id=self.id())

        log.info("Test tear down finished")


class RestartPerSuite(BaseTest):
    @classmethod
    def setUpClass(cls):
        log = logging.getLogger("BaseTest")
        log.info("Starting test setup")

        (cls.gpio, cls.system) = System.build_from_config(config)

        boot_wrappers = cls._get_class_boot_wrappers()

        cls.system.restart(boot_wrappers)

        log.info("Test setup finished")

    def setUp(self):
        self.system = self.__class__.system
        self.gpio = self.__class__.gpio

        extensions.set_up(test_id=self.id())

    def tearDown(self):
        extensions.tear_down(test_id=self.id())

    @classmethod
    def tearDownClass(cls):
        log = logging.getLogger("BaseTest")
        log.info("Starting test tear down")
        cls.system.obc.sync_fs()

        cls.system.close()
        cls.gpio.close()

        log.info("Test tear down finished")

