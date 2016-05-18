from Queue import Queue, Empty

import i2cMock
from threading import Event

class TransmitterDevice(i2cMock.I2CDevice):
    def __init__(self):
        super(TransmitterDevice, self).__init__(0x62)
        self._reset = Event()
        self._buffer = Queue()
    
    @i2cMock.command([0xAA])
    def _reset(self):
        print "transmitter reset"
        self._reset.set()

    @i2cMock.command([0x10])
    def _send_frame(self, *data):
        print "Send frame(%s)" % str(data)

        self._buffer.put_nowait(data)

    def wait_for_reset(self, timeout=None):
        return self._reset.wait(timeout)

    def get_message_from_buffer(self, timeout=None):
        return self._buffer.get(timeout=timeout)


class ReceiverDevice(i2cMock.I2CDevice):
    def __init__(self):
        super(ReceiverDevice, self).__init__(0x60)
        self._reset = Event()
        self._buffer = Queue()

    @i2cMock.command([0xAA])
    def _reset(self):
        print "receiver reset"
        self._reset.set()

    @i2cMock.command([0x21])
    def _get_number_of_frames(self):
        print "Get number of frames"
        return [self._buffer.qsize()]

    @i2cMock.command([0x22])
    def _receive_frame(self):
        try:
            frame = self._buffer.get_nowait()
        except Empty:
            return []

        return ReceiverDevice.build_frame_response(frame, 257, 300)

    @classmethod
    def build_frame_response(cls, content, doppler, rssi):
        length = len(content)

        length_bytes = [length & 0xFF, (length >> 8) & 0xFF]
        doppler_bytes = [doppler & 0xFF, (doppler >> 8) & 0xFF]
        rssi_bytes = [rssi & 0xFF, (rssi >> 8) & 0xFF]
        content_bytes = [ord(c) for c in content]

        return length_bytes + doppler_bytes + rssi_bytes + content_bytes

    def wait_for_reset(self, timeout=None):
        return self._reset.wait(timeout)

    def put_frame(self, data):
        self._buffer.put_nowait(data)