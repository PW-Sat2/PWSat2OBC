from Queue import Queue, Empty
import logging

import i2cMock
from i2cMock import I2CDevice
from threading import Event

EPS_DEVICE_ADDRESS = 12


class LCLTimeoutException(Exception):
    def __init__(self, desired):
        self.desired = desired

    def __str__(self):
        return "Timeout waiting for LCL to become %s" % (self.desired,)


class LCL:
    def __init__(self):
        self.on_event = Event()
        self.off_event = Event()
        self.changes = Queue()
        self.is_on = False
        self.was_on = False

    def on(self):
        self.is_on = True
        self.was_on = True

        self.on_event.set()
        self.off_event.clear()

        self.changes.put(True)

    def off(self):
        self.is_on = False

        self.on_event.clear()
        self.off_event.set()

        self.changes.put(False)

    def wait_for_change(self, timeout=None):
        try:
            return self.changes.get(True, timeout)
        except Empty:
            raise LCLTimeoutException("x")

    def wait_for_on(self, timeout=None):
        if not self.on_event.wait(timeout):
            raise LCLTimeoutException("on")

    def wait_for_off(self, timeout=None):
        if not self.off_event.wait(timeout):
            raise LCLTimeoutException("off")


class EPSDevice(I2CDevice):
    def __init__(self):
        super(EPSDevice, self).__init__(EPS_DEVICE_ADDRESS)

        self.log = logging.getLogger("EPS")

        self.sail0 = LCL()
        self.sail1 = LCL()
        self.power_cycle = Event()

    @i2cMock.command([0x01])
    def lcl_sail_0(self, onoff):
        if onoff == 1:
            self.sail0.on()
        else:
            self.sail0.off()

        self.log.debug("LCL_SAIL_0: %s" % str(onoff))

    @i2cMock.command([0x02])
    def lcl_sail_1(self, onoff):
        if onoff == 1:
            self.sail1.on()
        else:
            self.sail1.off()

        self.log.debug("LCL_SAIL_1: %s" % str(onoff))

    @i2cMock.command([0xA0])
    def trigger_system_power_cycle(self):
        self.log.info("Triggered system power cycle")
        self.power_cycle.set()

    def wait_for_sail_open(self):
        self.sail0.wait_for_on(1)
        self.sail0.wait_for_off(1)
        self.sail1.wait_for_on(1)
        self.sail1.wait_for_off(1)
