from Queue import Queue

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

