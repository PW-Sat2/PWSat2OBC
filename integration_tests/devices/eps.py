import struct
from Queue import Queue, Empty
import logging

from datetime import datetime

import i2cMock
from i2cMock import I2CDevice
from threading import Event

from utils import call, CompareAsDict, bitlist_to_byte


class MPPT(CompareAsDict):
    def __init__(self):
        self.SOL_VOLT = 0
        self.SOL_CURR = 0
        self.SOL_OUT_VOLT = 0
        self.TEMP = 0
        self.STATE = 0

    def bytes(self):
        return list(struct.pack('<HHHHB', self.SOL_CURR, self.SOL_VOLT, self.SOL_OUT_VOLT, self.TEMP, self.STATE))


class DISTR(CompareAsDict):
    def __init__(self):
        self.VOLT_3V3 = 0
        self.CURR_3V3 = 0
        self.VOLT_5V = 0
        self.CURR_5V = 0
        self.VOLT_VBAT = 0
        self.CURR_VBAT = 0
        self.LCL_STATE = 0
        self.LCL_FLAGB = 0

    def bytes(self):
        return list(struct.pack('<HHHHHHBB',
                                self.CURR_3V3,
                                self.VOLT_3V3,
                                self.CURR_5V,
                                self.VOLT_5V,
                                self.CURR_VBAT,
                                self.VOLT_VBAT,
                                self.LCL_STATE,
                                self.LCL_FLAGB
                                ))


class BATC_A(CompareAsDict):
    def __init__(self):
        self.VOLT_A = 0
        self.CHRG_CURR = 0
        self.DCHRG_CURR = 0
        self.TEMP = 0
        self.STATE = 0

    def bytes(self):
        return list(struct.pack('<HHHHB',
                                self.VOLT_A,
                                self.CHRG_CURR,
                                self.DCHRG_CURR,
                                self.TEMP,
                                self.STATE))


class BATC_B(CompareAsDict):
    def __init__(self):
        self.VOLT_B = 0

    def bytes(self):
        return list(struct.pack('<H', self.VOLT_B))


class BP_A(CompareAsDict):
    def __init__(self):
        self.TEMP_A = 0
        self.TEMP_B = 0

    def bytes(self):
        return list(struct.pack('<HH', self.TEMP_A, self.TEMP_B))


class BP_B(CompareAsDict):
    def __init__(self):
        self.TEMP_C = 0

    def bytes(self):
        return list(struct.pack('<H', self.TEMP_C))


class OtherController(CompareAsDict):
    def __init__(self):
        self.VOLT_3V3d = 0

    def bytes(self):
        return list(struct.pack('<H', self.VOLT_3V3d))


class ThisController(CompareAsDict):
    def __init__(self):
        self.SAFETY_CTR = 0
        self.PWR_CYCLES = 0
        self.UPTIME = 0
        self.TEMP = 0
        self.SUPP_TEMP = 0

    def bytes(self):
        return list(struct.pack('<BHIHH',
                                self.SAFETY_CTR,
                                self.PWR_CYCLES,
                                self.UPTIME,
                                self.TEMP,
                                self.SUPP_TEMP))


class DCDC(CompareAsDict):
    def __init__(self):
        self.TEMP = 0

    def bytes(self):
        return list(struct.pack('<H', self.TEMP))


class HousekeepingA(CompareAsDict):
    def __init__(self):
        self.MPPT_X = MPPT()
        self.MPPT_Y_PLUS = MPPT()
        self.MPPT_Y_MINUS = MPPT()
        self.DISTR = DISTR()
        self.BATC = BATC_A()
        self.BP = BP_A()
        self.CTRLB = OtherController()
        self.CTRLA = ThisController()
        self.DCDC3V3 = DCDC()
        self.DCDC5V = DCDC()

    def bytes(self):
        return self.MPPT_X.bytes() \
               + self.MPPT_Y_PLUS.bytes() \
               + self.MPPT_Y_MINUS.bytes() \
               + self.DISTR.bytes() \
               + self.BATC.bytes() \
               + self.BP.bytes() \
               + self.CTRLB.bytes() \
               + self.CTRLA.bytes() \
               + self.DCDC3V3.bytes() \
               + self.DCDC5V.bytes()


class HousekeepingB(CompareAsDict):
    def __init__(self):
        self.BP = BP_B()
        self.BATC = BATC_B()
        self.CTRLA = OtherController()
        self.CTRLB = ThisController()

    def bytes(self):
        return sum(map(lambda x: x.bytes(), [self.BP, self.BATC, self.CTRLA, self.CTRLB]), [])


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
        self.on_disable = None

    def arrange_on(self):
        self.is_on = True

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
        call(self.on_disable, True)

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
        self.enabled = False
        self.on_enable = None

    def enable(self):
        self.enabled = True
        call(self.on_enable, True)


class EPSControllerA(I2CDevice):
    def __init__(self, eps):
        super(EPSControllerA, self).__init__(0x35, "EPS.A")
        self._eps = eps

        self._lcls = [eps.TKmain, eps.SunS, eps.CamNadir, eps.CamWing, eps.SENS, eps.ANTenna]
        self._burn_switches = [eps.SAILmain, eps.SADSmain]

        self.on_power_cycle = None
        self.on_disable_overheat_submode = None
        self.on_get_housekeeping = None
        self.on_reset_watchdog = None

        self.hk = HousekeepingA()

        self.last_watchdog_kick = None

    @i2cMock.command([0xE0])
    def _power_cycle(self):
        self.log.info("Triggered power cycle")
        call(self.on_power_cycle, None)

    @i2cMock.command([0xE1])
    def _enable_lcl(self, lcl_id):
        self.log.info("Enable LCL(%d)", lcl_id)
        self._lcls[lcl_id - 1].on()

    @i2cMock.command([0xE2])
    def _disable_lcl(self, lcl_id):
        self.log.info("Disable LCL(%d)", lcl_id)
        self._lcls[lcl_id - 1].off()

    @i2cMock.command([0xE3])
    def _enable_burn_switch(self, switch_id):
        self.log.info("Enable BURN switch(%d)", switch_id)
        self._burn_switches[switch_id - 1].enable()

    @i2cMock.command([0xE4])
    def _disable_overheat_submode(self):
        self.log.info("Disable overheat submode")
        call(self.on_disable_overheat_submode, None)

    @i2cMock.command([0xE5])
    def _reset_watchdog(self):
        self.log.info("Resetting watchdog")
        self.last_watchdog_kick = datetime.now()
        call(self.on_reset_watchdog, None)

    @i2cMock.command([0x0])
    def _housekeeping(self):
        self.hk.DISTR.LCL_STATE = bitlist_to_byte(map(lambda lcl: 1 if lcl.is_on else 0, self._lcls))
        self.hk.DISTR.LCL_FLAGB = bitlist_to_byte(map(lambda lcl: 1 if lcl.is_on else 0, self._lcls))

        hk = call(self.on_get_housekeeping, default=self.hk)
        return [0x61] + hk.bytes()

    @i2cMock.command([0x4B])
    def _error_code(self):
        hk = call(self.on_get_housekeeping, default=self.hk)
        return hk.bytes()[0x4B - 1:]


class EPSControllerB(I2CDevice):
    def __init__(self, eps):
        super(EPSControllerB, self).__init__(0x36, "EPS.B")
        self._eps = eps

        self._lcls = [eps.TKred, eps.ANTennaRed]
        self._burn_switches = [eps.SAILred, eps.SADSred]

        self.on_power_cycle = None
        self.on_disable_overheat_submode = None
        self.on_get_housekeeping = None
        self.on_reset_watchdog = None

        self.hk = HousekeepingB()

        self.last_watchdog_kick = None

    @i2cMock.command([0xE0])
    def _power_cycle(self):
        self.log.info("Triggered power cycle")
        call(self.on_power_cycle, None)

    @i2cMock.command([0xE1])
    def _enable_lcl(self, lcl_id):
        self.log.info("Enable LCL(%d)", lcl_id)
        self._lcls[lcl_id - 1].on()

    @i2cMock.command([0xE2])
    def _disable_lcl(self, lcl_id):
        self.log.info("Disable LCL(%d)", lcl_id)
        self._lcls[lcl_id - 1].off()

    @i2cMock.command([0xE3])
    def _enable_burn_switch(self, switch_id):
        self.log.info("Enable BURN switch(%d)", switch_id)
        self._burn_switches[switch_id - 1].enable()

    @i2cMock.command([0xE4])
    def _disable_overheat_submode(self):
        self.log.info("Disable overheat submode")
        call(self.on_disable_overheat_submode, None)

    @i2cMock.command([0xE5])
    def _reset_watchdog(self):
        self.log.info("Resetting watchdog")
        self.last_watchdog_kick = datetime.now()
        call(self.on_reset_watchdog, None)

    @i2cMock.command([0x0])
    def _housekeeping(self):
        hk = call(self.on_get_housekeeping, default=self.hk)
        return [0x9D] + hk.bytes()

    @i2cMock.command([0x07])
    def _error_code(self):
        hk = call(self.on_get_housekeeping, default=self.hk)
        return hk.bytes()[0x07 - 1:]


class EPS:
    ERROR_COUNTER = 1

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

        self.controller_a = EPSControllerA(self)
        self.controller_b = EPSControllerB(self)
