from Queue import Queue, Empty
import logging

import i2cMock
from i2cMock import I2CDevice
from threading import Event

from utils import call

EPS_DEVICE_ADDRESS = 0x35


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
        self.on_enable = None

    def on(self):
        self.is_on = True
        self.was_on = True

        self.on_event.set()
        self.off_event.clear()

        self.changes.put(True)
        call(self.on_enable, True)

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


class BurnSwitch:
    def __init__(self):
        self.on_enable = None

    def enable(self):
        call(self.on_enable, True)


class EPSControllerA(I2CDevice):
    def __init__(self, eps):
        super(EPSControllerA, self).__init__(0x35)
        self._log = logging.getLogger("EPS.B")
        self._eps = eps

        self._lcls = [eps.TKmain, eps.SunS, eps.CamNadir, eps.CamWing, eps.SENS, eps.ANTenna]
        self._burn_switches = [eps.SAILmain, eps.SADSmain]

    @i2cMock.command([0xE0])
    def _power_cycle(self):
        self._log.info("Triggered power cycle")
        self._eps.power_cycle()

    @i2cMock.command([0xE1])
    def _enable_lcl(self, lcl_id):
        self._log.info("Enable LCL(%d)", lcl_id)
        self._lcls[lcl_id - 1].on()

    @i2cMock.command([0xE2])
    def _disable_lcl(self, lcl_id):
        self._log.info("Disable LCL(%d)", lcl_id)
        self._lcls[lcl_id - 1].off()

    @i2cMock.command([0xE3])
    def _enable_burn_switch(self, switch_id):
        self._log.info("Enable BURN switch(%d)", switch_id)
        self._burn_switches[switch_id - 1].enable()


class EPSControllerB(I2CDevice):
    def __init__(self, eps):
        super(EPSControllerB, self).__init__(0x36)
        self._log = logging.getLogger("EPS.A")
        self._eps = eps

        self._lcls = [eps.TKred, eps.ANTennaRed]
        self._burn_switches = [eps.SAILred, eps.SADSred]

    @i2cMock.command([0xE0])
    def _power_cycle(self):
        self._log.info("Triggered power cycle")
        self._eps.power_cycle()

    @i2cMock.command([0xE1])
    def _enable_lcl(self, lcl_id):
        self._log.info("Enable LCL(%d)", lcl_id)
        self._lcls[lcl_id - 1].on()

    @i2cMock.command([0xE2])
    def _disable_lcl(self, lcl_id):
        self._log.info("Disable LCL(%d)", lcl_id)
        self._lcls[lcl_id - 1].off()

    @i2cMock.command([0xE3])
    def _enable_burn_switch(self, switch_id):
        self._log.info("Enable BURN switch(%d)", switch_id)
        self._burn_switches[switch_id - 1].enable()


class EPS:
    def __init__(self):
        self.TKmain = LCL()
        self.SunS = LCL()
        self.CamNadir = LCL()
        self.CamWing = LCL()
        self.SENS = LCL()
        self.ANTenna = LCL()
        self.TKred = LCL()
        self.ANTennaRed = LCL()

        self.SAILmain = BurnSwitch()
        self.SAILred = BurnSwitch()
        self.SADSmain = BurnSwitch()
        self.SADSred = BurnSwitch()

        self.on_power_cycle = None

        self.controller_a = EPSControllerA(self)
        self.controller_b = EPSControllerB(self)

    def power_cycle(self):
        call(self.on_power_cycle, None)
