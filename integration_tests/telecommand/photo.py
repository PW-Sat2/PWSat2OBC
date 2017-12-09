from telecommand import CorrelatedTelecommand
import struct


class TakePhotoTelecommand(CorrelatedTelecommand):
    def __init__(self, correlation_id, camera, resolution, count, delay, path):
        CorrelatedTelecommand.__init__(self, correlation_id)
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

    def __repr__(self):
        return "{}, camera={}, picture={}".format(
            super(TakePhotoTelecommand, self).__repr__(),
            self._camera_id,
            self._picture_path)


class PurgePhotoTelecommand(CorrelatedTelecommand):
    def __init__(self, correlation_id):
        CorrelatedTelecommand.__init__(self, correlation_id)

    def apid(self):
        return 0x22

    def payload(self):
        return struct.pack('<B', self._correlation_id)
