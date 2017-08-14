import logging
import time
from datetime import datetime

from nose.tools import nottest

from system import runlevel
from tests.base import RestartPerTest
from i2cMock import MockPin


class TestCamera(RestartPerTest):
    @runlevel(1)
    @nottest
    def test_camera(self):
        log = logging.getLogger("test_camera")
        log.info("Enable CAM LCL")
        self.system.camera.enable_nadir()
        self.system.camera.enable_wing()

        log.info("Sync")
        self.system.obc.camera_sync()
        time.sleep(3)

        log.info("Take")

        camera_file_name = 'camera_{:%Y%m%d%H%M%f}.dat'.format(datetime.now());

        self.system.obc.camera_take_jpeg_to_file("240", camera_file_name)
        self.system.camera.decode_photo(camera_file_name)

        log.info("Done")
        self.system.camera.disable_nadir()
        self.system.camera.disable_wing()


