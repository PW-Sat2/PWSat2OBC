import logging

from obc import OBC, SerialPortTerminal
from devices import *
from i2cMock import I2CMock


class System:
    def __init__(self, obc_com, sys_bus_com, payload_bus_com, use_single_bus):
        self.log = logging.getLogger("system")

        self.obc_com = obc_com
        self.sys_bus_com = sys_bus_com
        self.payload_bus_com = payload_bus_com

        self.sys_bus = I2CMock(sys_bus_com)

        if use_single_bus:
            self.payload_bus = self.i2c
        else:
            self.payload_bus = I2CMock(payload_bus_com)

        self._setup_devices()

        self.obc = OBC(SerialPortTerminal(obc_com))
        self.obc.power_off()

        self.sys_bus.start()
        self.payload_bus.start()

        self.obc.power_on()

    def _setup_devices(self):
        self.eps = EPSDevice()
        self.transmitter = TransmitterDevice()
        self.receiver = ReceiverDevice()

        self.sys_bus.add_device(self.eps)
        self.sys_bus.add_device(self.transmitter)
        self.sys_bus.add_device(self.receiver)

    def close(self):
        self.sys_bus.close()
        self.payload_bus.close()

        self.obc.close()
