import logging
import time
from datetime import datetime

from nose.tools import nottest

from system import runlevel
from tests.base import RestartPerTest

class TestCamera(RestartPerTest):
    @runlevel(1)
    @nottest
    def test_camera(self):
        log = logging.getLogger("test_camera")

        log.info("Sync")
        self.system.obc.sync()
        time.sleep(3)

        log.info("Take")

        camera_file_name = 'camera_{:%Y%m%d%H%M%f}.dat'.format(datetime.now());

        self.system.obc.takeJpegToFile("240", camera_file_name)
        self.decode_photo(camera_file_name)

        log.info("Done")

    def decode_photo(self, localName):
        result = []

        with open(localName, 'rb') as f:
            data = f.read()

        rem = data[4:]

        while len(rem) > 0:
            part = rem[0:512 - 6]

            result += part

            rem = rem[512:]

        with open(localName + '.jpg', 'wb') as f:
            f.write(''.join(result))
