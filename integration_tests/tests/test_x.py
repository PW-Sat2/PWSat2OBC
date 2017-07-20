from datetime import datetime

import logging

from nose.tools import nottest

from tests.base import RestartPerTest

# @nottest
class XTest(RestartPerTest):
    def test_x(self):
        log = logging.getLogger("test")
        logging.getLogger("I2C").propagate = False

        counter = 0

        while True:
            buffer = ''.join([datetime.now().isoformat()] * 1)

            log.info("WRITE %d", counter)
            self.system.obc.write_file("/test", buffer)
            log.info("AFTER WRITE %d", counter)

            log.info("READ %d", counter)
            read_back = self.system.obc.read_file("/test")
            log.info("AFTER READ %d", counter)

            # self.assertEqual(buffer, read_back)

            # pong = self.system.obc.ping()
            # self.assertEqual(pong, 'pong')

            counter += 1
