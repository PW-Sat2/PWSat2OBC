from telecommand import Telecommand
import struct


class TakePhotoTelecommand(Telecommand):
    def __init__(self, correlation_id, camera, resolution, count, delay, path):
        Telecommand.__init__(self)
        self._correlation_id = correlation_id
        self._camera_id = camera
        self._resolution = resolution
        self._picture_count = count
        self._delay = delay
        self._picture_path = path[:30]

    def apid(self):
        return 0x1F

    def payload(self):
        return struct.pack('<BBBBH',
                           self._correlation_id,
                           self._camera_id.value,
                           self._resolution.value,
                           self._picture_count,
                           self._delay.total_seconds(),
                           ) + self._picture_path + '\0'