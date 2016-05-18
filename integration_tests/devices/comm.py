from Queue import Queue, Empty

import i2cMock
from threading import Event

counter = 0


class TransmitterDevice(i2cMock.I2CDevice):
    MAX_CONTENT_SIZE = 235
    BUFFER_SIZE = 40

    def __init__(self):
        super(TransmitterDevice, self).__init__(0x62)
        self._reset = Event()
        self._buffer = Queue(TransmitterDevice.BUFFER_SIZE)
    
    @i2cMock.command([0xAA])
    def _reset(self):
        print "transmitter reset"
        self._reset.set()

    @i2cMock.command([0x10])
    def _send_frame(self, *data):
        print "Send frame(%s)" % str(data)

        self._buffer.put_nowait(data)

        return [TransmitterDevice.BUFFER_SIZE - self._buffer.qsize()]

    def wait_for_reset(self, timeout=None):
        return self._reset.wait(timeout)

    def get_message_from_buffer(self, timeout=None):
        return self._buffer.get(timeout=timeout)


class ReceiverDevice(i2cMock.I2CDevice):
    def __init__(self):
        global  counter
        print "ReceiverDevice.__init__"
        super(ReceiverDevice, self).__init__(0x60)
        self._reset = Event()
        self._buffer = Queue()
        counter += 1

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
        print "Receive frame %d" % counter

        if self._buffer.empty():
            return []

        frame = self._buffer.queue[0]

        return ReceiverDevice.build_frame_response(frame, 257, 300)

    @i2cMock.command([0x24])
    def _remove_frame(self):
        try:
            self._buffer.get_nowait()
        except Empty:
            pass

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

    def queue_size(self):
        return self._buffer.qsize()
