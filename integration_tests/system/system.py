import logging

from obc import OBC, SerialPortTerminal
from devices import *
from i2cMock import I2CMock
from pins import Pins


class System:
    def __init__(self, obc_com, sys_bus_com, payload_bus_com, use_single_bus, gpio, auto_power_on = True):
        self.log = logging.getLogger("system")

        self._use_single_bus = use_single_bus

        self.obc_com = obc_com
        self.sys_bus_com = sys_bus_com
        self.payload_bus_com = payload_bus_com

        self.sys_bus = I2CMock('SYS', sys_bus_com)

        if use_single_bus:
            self.payload_bus = self.sys_bus
        else:
            self.payload_bus = I2CMock('PLD', payload_bus_com)

        self._setup_devices()

        self.obc = OBC(SerialPortTerminal(obc_com, gpio))
        self.obc.power_off()

        self.sys_bus.start()
        self.payload_bus.start()

        if auto_power_on:
            self.obc.power_on(clean_state=True)

    def _setup_devices(self):
        self.eps = EPSDevice()
        self.transmitter = TransmitterDevice()
        self.receiver = ReceiverDevice()
        self.primary_antenna = AntennaController(PRIMARY_ANTENNA_CONTROLLER_ADDRESS)
        self.backup_antenna = AntennaController(BACKUP_ANTENNA_CONTROLLER_ADDRESS)

        self.sys_bus.add_device(self.eps)
        self.sys_bus.add_device(self.transmitter)
        self.sys_bus.add_device(self.receiver)
        self.sys_bus.add_device(self.primary_antenna)
        self.sys_bus.add_device(self.backup_antenna)

    def close(self):
        self.sys_bus.stop()
        if not self._use_single_bus:
            self.payload_bus.stop()

        self.obc.close()

    def restart(self):
        self.sys_bus.unfreeze()
        self.sys_bus.unlatch()

        if not self._use_single_bus:
            self.payload_bus.unfreeze()
            self.payload_bus.unlatch()

        self.obc.reset()
        self.obc.wait_to_start()
