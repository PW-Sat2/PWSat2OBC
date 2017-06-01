import logging

from nose.tools import nottest
from nose_parameterized import parameterized

from system import wait_for_obc_start
from datetime import datetime, timedelta
from tests.base import BaseTest
import time
from unittest import skip

class TestTMR(BaseTest):
    @wait_for_obc_start()
    @skip('Manual test')
    def test_success_when_single_erase(self):
        log = logging.getLogger("test_tmr")
        self.system.obc.write_file("/a/tmr.test", "PW-SAT2")

        log.info("Erasing 1 ...")
        self.system.obc.erase(1)
        test_file_content = self.system.obc.read_file("/a/tmr.test")

        self.assertEquals("PW-SAT2", test_file_content)

    @wait_for_obc_start()
    def test_failure_when_double_erase(self):
        log = logging.getLogger("test_tmr")

        self.system.obc.write_file("/a/tmr.test", "Should not be PW-SAT2")

        log.info("Erasing 1 ...")
        self.system.obc.erase(1)
        log.info("Erasing 2 ...")
        self.system.obc.erase(2)
        log.info("Erased")

        test_file_content = self.system.obc.read_file("/a/tmr.test")

        self.assertNotEqual("Should not be PW-SAT2", test_file_content)

        self.system.restart()

    @wait_for_obc_start()
    def test_fram_redundant_read_write(self):
        self.system.obc.fram_write('r', 0, ['1', '2', '3'])

        fram_value = self.system.obc.fram_read('r', 0, 3)

        self.assertEquals(['1', '2', '3'], fram_value)

    @wait_for_obc_start()
    def test_fram_single_failure(self):
        self.system.obc.fram_write('r', 0, ['1', '2', '3'])
        self.system.obc.fram_write('0', 0, ['6', '6', '6'])

        fram_value = self.system.obc.fram_read('r', 0, 3)

        self.assertEquals(['1', '2', '3'], fram_value)

    @wait_for_obc_start()
    def test_fram_double_failure(self):
        self.system.obc.fram_write('r', 0, ['1', '2', '3'])
        self.system.obc.fram_write('0', 0, ['6', '6', '6'])
        self.system.obc.fram_write('1', 0, ['6', '6', '6'])

        fram_value = self.system.obc.fram_read('r', 0, 3)

        self.assertEquals(['6', '6', '6'], fram_value)
