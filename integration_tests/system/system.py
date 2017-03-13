import logging

from devices import *
from i2cMock import I2CMock
from obc import OBC, SerialPortTerminal


class System:
    def __init__(self, obc_com, mock_com, gpio, auto_power_on=True):
        self.log = logging.getLogger("system")

        self.obc_com = obc_com
        self.mock_com = mock_com

        self.i2c = I2CMock(mock_com)

        self._setup_devices()

        self.obc = OBC(SerialPortTerminal(obc_com, gpio))
        self.obc.power_off()

        self.i2c.start()

        if auto_power_on:
            self.obc.power_on(clean_state=True)

    def _setup_devices(self):
        self.eps = EPS()
        self.comm = Comm()
        self.transmitter = self.comm.transmitter
        self.receiver = self.comm.receiver
        self.primary_antenna = AntennaController(PRIMARY_ANTENNA_CONTROLLER_ADDRESS)
        self.backup_antenna = AntennaController(BACKUP_ANTENNA_CONTROLLER_ADDRESS)
        self.imtq = Imtq()
        self.rtc = RTCDevice()

        self.i2c.add_bus_device(self.eps.controller_a)
        self.i2c.add_pld_device(self.eps.controller_b)
        self.i2c.add_bus_device(self.transmitter)
        self.i2c.add_bus_device(self.receiver)
        self.i2c.add_bus_device(self.primary_antenna)
        self.i2c.add_pld_device(self.backup_antenna)
        self.i2c.add_bus_device(self.imtq)
        self.i2c.add_pld_device(self.rtc)

    def close(self):
        self.i2c.stop()
        self.obc.close()

    def restart(self):
        self.i2c.unlatch()
        self.obc.reset()
        self.obc.wait_to_start()
