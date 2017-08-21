from response_frames.operation import OperationSuccessFrame, OperationErrorFrame
from system import auto_power_on, clear_state
from telecommand import TakePhotoTelecommand, PurgePhotoTelecommand
from tests.base import RestartPerTest
from utils import TestEvent
from devices import CameraLocation, PhotoResolution
from datetime import timedelta


class TestPhotoTelecommand(RestartPerTest):
    @auto_power_on(auto_power_on=False)
    def __init__(self, *args, **kwargs):
        super(TestPhotoTelecommand, self).__init__(*args, **kwargs)

    def _start(self):
        e = TestEvent()

        def on_reset(_):
            e.set()

        self.system.comm.on_hardware_reset = on_reset

        self.power_on_obc()

        e.wait_for_change(1)

    def test_photo_telecommand_invalid_picture_count(self):
        self._start()

        self.system.comm.put_frame(TakePhotoTelecommand(10, CameraLocation.Wing, PhotoResolution.p128, 30, timedelta(seconds = 0), "photo.jpg"))

        ack = self.system.comm.get_frame(5)
        self.assertIsInstance(ack, OperationErrorFrame)

    def test_photo_telecommand(self):
        self._start()

        self.system.comm.put_frame(TakePhotoTelecommand(10, CameraLocation.Wing, PhotoResolution.p128, 10, timedelta(seconds = 5), "photo.jpg"))

        ack = self.system.comm.get_frame(5)
        self.assertIsInstance(ack, OperationSuccessFrame)

    def test_purge_photo_telecommand(self):
        self._start()

        self.system.comm.put_frame(PurgePhotoTelecommand(10))

        ack = self.system.comm.get_frame(10)
        self.assertIsInstance(ack, OperationSuccessFrame)
