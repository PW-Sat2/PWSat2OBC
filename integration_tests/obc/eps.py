import re
from enum import Enum

from obc import OBCMixin
from .obc_mixin import command, decode_return, decode_bool
from devices.eps import HousekeepingA, HousekeepingB


class PowerCycleBy(Enum):
    A = 'A'
    B = 'B'
    Auto = 'Auto'


class ResetWatchdogOn(Enum):
    A = 'A'
    B = 'B'
    Both = 'Both'


class EPSMixin(OBCMixin):
    @decode_return(int)
    @command("eps enable_lcl {0}")
    def enable_lcl(self, lcl):
        pass

    @decode_return(int)
    @command("eps disable_lcl {0}")
    def disable_lcl(self, lcl):
        pass

    @decode_bool()
    def power_cycle(self, by):
        self._command_no_wait("eps power_cycle {0!n}", by)

    @decode_bool()
    @command("eps disable_overheat {0}")
    def disable_overheat_submode(self, controller):
        pass

    @decode_return(int)
    @command("eps enable_burn_switch {0:d} {1}")
    def enable_burn_switch(self, main, switch):
        pass

    def _parse_hk(type):
        def parse(s):
            hk = type()

            lines = s.split('\n')
            for line in lines:
                (group, key, value) = re.split('\.|\t', line)

                setattr(getattr(hk, group), key, int(value))

            return hk
        return parse

    @decode_return(_parse_hk(HousekeepingA))
    @command("eps hk_a")
    def read_housekeeping_a(self):
        pass

    @decode_return(_parse_hk(HousekeepingB))
    @command("eps hk_b")
    def read_housekeeping_b(self):
        pass

    @decode_return(int)
    @command("eps reset_watchdog {0!n}")
    def eps_reset_watchdog(self, controller):
        pass
