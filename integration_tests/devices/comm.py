import logging
import i2cMock
import time
from Queue import Queue, Empty
from utils import *

class TransmitterDevice(i2cMock.I2CDevice):
    MAX_CONTENT_SIZE = 235
    BUFFER_SIZE = 40

    def __init__(self):
        super(TransmitterDevice, self).__init__(0x62)
        self.log = logging.getLogger("Comm Transmitter")
        self.on_watchdog_reset = None
        self.on_hardware_reset = None
        self.on_reset = None
        self.on_send_frame = None
        self._buffer = Queue(TransmitterDevice.BUFFER_SIZE)
    
    @i2cMock.command([0xAA])
    def _reset(self):
        self.log.info("Reset")
        call(self.on_reset, None)
        reset()

    @i2cMock.command([0xAB])
    def _hwreset(self):
        self.log.info("Hardware Reset")
        call(self.on_hardware_reset, None)

    @i2cMock.command([0xCC])
    def _watchdog_reset(self):
        self.log.info("Watchdog Reset")
        call(self.on_watchdog_reset, None)

    @i2cMock.command([0x10])
    def _send_frame(self, *data):
        self.log.info("Send frame %s", data)

        call(self.on_send_frame, None, self, data)
        self._buffer.put_nowait(data)
        return [TransmitterDevice.BUFFER_SIZE - self._buffer.qsize()]

    def get_message_from_buffer(self, timeout=None):
        return self._buffer.get(timeout=timeout)

    def reset(self):
        self._buffer = Queue(TransmitterDevice.BUFFER_SIZE)

class ReceiverDevice(i2cMock.I2CDevice):
    def __init__(self):
        super(ReceiverDevice, self).__init__(0x60)
        self.log = logging.getLogger("Comm Receiver")
        self.on_watchdog_reset = None
        self.on_hardware_reset = None
        self.on_reset = None
        self.on_frame_remove = None
        self.on_frame_receive = None
        self._buffer = Queue()

    @i2cMock.command([0xAA])
    def _reset(self):
        self.log.info("Reset")
        call(self.on_reset, None)
        reset()

    @i2cMock.command([0xAB])
    def _hwreset(self):
        self.log.info("Hardware reset")
        call(self.on_hardware_reset, None)

    @i2cMock.command([0xCC])
    def _watchdog_reset(self):
        self.log.info("Watchdog Reset")
        call(self.on_watchdog_reset, None)

    @i2cMock.command([0x21])
    def _get_number_of_frames(self):
        size = self._buffer.qsize();
        return [size & 0xff, size / 256]

    @i2cMock.command([0x22])
    def _receive_frame(self):
        if call(self.on_frame_receive, True, self):
            if self._buffer.empty():
                return []

            frame = self._buffer.queue[0]
            return ReceiverDevice.build_frame_response(frame, 257, 300)
        else:
            return []

    @i2cMock.command([0x24])
    def _remove_frame(self):
        try:
            if call(self.on_frame_remove, True, self):
                self._buffer.get_nowait()
        except Empty:
            pass

    def reset(self):
        self._buffer = Queue()

    @classmethod
    def build_frame_response(cls, content, doppler, rssi):
        length = len(content)

        length_bytes = [length & 0xFF, (length >> 8) & 0xFF]
        doppler_bytes = [doppler & 0xFF, (doppler >> 8) & 0xFF]
        rssi_bytes = [rssi & 0xFF, (rssi >> 8) & 0xFF]
        content_bytes = [ord(c) for c in content]

        return length_bytes + doppler_bytes + rssi_bytes + content_bytes

    def put_frame(self, data):
        self._buffer.put_nowait(data)

    def queue_size(self):
        return self._buffer.qsize()

class Comm(object):
    def __init__(self):
        self.transmitter = TransmitterDevice()
        self.receiver = ReceiverDevice()
        self.on_hardware_reset = None
        self.on_watchdog_reset = None
        self.transmitter.on_hardware_reset = self._hwreset
        self.transmitter.on_watchdog_reset = self._watchdog_reset
        self.receiver.on_hardware_reset = self._hwreset
        self.receiver.on_watchdog_reset = self._watchdog_reset

    def _hwreset(self):
        if call(self.on_hardware_reset, True, self):
            self.transmitter.reset()
            self.receiver.reset()

    def _watchdog_reset(self):
        call(self.on_watchdog_reset, None, self)
