from datetime import datetime, timedelta
import logging
from base64 import b64encode

from utils import ExtendableFormatter
from .antenna import AntennaMixin
from .comm import CommMixin
from .experiments import ExperimentsMixin
from .file_system import FileSystemMixin
from .i2c import I2CMixin
from .imtq import ImtqMixin
from .suns import SunSMixin
from .gyro import GyroMixin
from .mission import MissionMixin
from .obc_mixin import OBCMixin
from .obc_time import TimeMixin
from .eps import EPSMixin
from .watchdog import WatchdogMixin
from .state import StateMixin
from .error_counters import ErrorCountersMixin
from .fram import FRAMMixin
from .boot_settings import BootSettingsMixin
from .payload import PayloadMixin
from .runlevel import RunlevelMixin
from .camera import CameraMixin
from .adcs import ADCSMixin
from .obc_rtos import RTOSMixin
from .mcu_temp import MCUTemperatureMixin
from .flash import FlashMixin


class OBC(OBCMixin,
          FileSystemMixin,
          CommMixin,
          TimeMixin,
          I2CMixin,
          AntennaMixin,
          MissionMixin,
          ImtqMixin,
          SunSMixin,
          GyroMixin,
          ExperimentsMixin,
          EPSMixin,
          WatchdogMixin,
          StateMixin,
          ErrorCountersMixin,
          FRAMMixin,
          BootSettingsMixin,
          PayloadMixin,
          RunlevelMixin,
          CameraMixin,
          ADCSMixin,
          RTOSMixin,
          MCUTemperatureMixin,
          FlashMixin
          ):
    def __init__(self, terminal):
        self.log = logging.getLogger("OBC")

        self._formatter = ExtendableFormatter()
        self._formatter.register_conversion('E', lambda d: b64encode(d).rstrip('='))
        self._formatter.register_conversion('n', lambda d: d.name)
        self._formatter.register_conversion('t', self._format_timedelta)

        self._terminal = terminal

    def _command(self, cmd, *args, **kwargs):
        cmdline = self._formatter.vformat(cmd, args, kwargs)

        return self._terminal.command(cmdline)

    def _command_no_wait(self, cmd, *args, **kwargs):
        cmdline = self._formatter.vformat(cmd, args, kwargs)

        return self._terminal.command_no_wait(cmdline)

    def _format_timedelta(self, d):
        if type(d) is int:
            return str(d)

        if type(d) is timedelta:
            return str(int(d.total_seconds() * 1000))

        raise TypeError('Must be int or timedelta')

    def wait_to_start(self):
        self.log.debug("Waiting for OBC initialization finish")

        start = datetime.now()

        self._command("wait_for_init")

        end = datetime.now()
        duration = end - start
        self.log.info("OBC initialization done in %s", str(duration))

    def reset(self, boot_handler):
        self._terminal.reset(boot_handler)

    def close(self):
        self._terminal.close()

    def power_off(self):
        self._terminal.power_off()

    def power_on(self, boot_handler):
        self._terminal.power_on(boot_handler)

    def ping(self):
        return self._command("ping")

    def compile_info(self):
        return self._command("compile_info")

    def wait_for_boot(self, timeout=None):
        return self._terminal.wait_for_boot(timeout)
