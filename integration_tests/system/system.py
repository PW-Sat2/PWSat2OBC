import logging

from obc import OBC, SerialPortTerminal
from devices import *
from i2cMock import I2CMock


class System:
    def __init__(self, mock_com, obc_com):
        self.log = logging.getLogger("system")

        self.obc_com = obc_com
        self.mock_com = mock_com

        self.i2c = I2CMock(mock_com)

        self._setup_devices()

        self.obc = OBC(SerialPortTerminal(obc_com))
        print "a"
        self.obc.power_off()

        self.i2c.start()

        self.obc.power_on()

    def _setup_devices(self):
        self.eps = EPSDevice()
        self.transmitter = TransmitterDevice()
        self.receiver = ReceiverDevice()

        self.i2c.add_device(self.eps)
        self.i2c.add_device(self.transmitter)
        self.i2c.add_device(self.receiver)

    def close(self):
        self.i2c.close()
        self.obc.close()
