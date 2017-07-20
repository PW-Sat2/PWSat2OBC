import logging
from datetime import datetime
from unittest import skip

from tests.base import RestartPerTest


class DMAStressTest(RestartPerTest):
    @skip('Unending test')
    def test_infinite_write(self):
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

            self.assertEqual(buffer, read_back)

            counter += 1
