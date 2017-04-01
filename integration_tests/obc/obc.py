import logging
from base64 import b64encode

import time

from utils import ExtendableFormatter

from .experiments import ExperimentsMixin
from .obc_mixin import OBCMixin
from .file_system import FileSystemMixin
from .antenna import AntennaMixin
from .comm import CommMixin
from .obc_time import TimeMixin
from .i2c import I2CMixin
from .mission import MissionMixin
from .imtq import ImtqMixin
from .eps import EPSMixin


class OBC(OBCMixin,
          FileSystemMixin,
          CommMixin,
          TimeMixin,
          I2CMixin,
          AntennaMixin,
          MissionMixin,
          ImtqMixin,
          ExperimentsMixin,
          EPSMixin
          ):
    def __init__(self, terminal):
        self.log = logging.getLogger("OBC")

        self._formatter = ExtendableFormatter()
        self._formatter.register_conversion('E', lambda d: b64encode(d).rstrip('='))
        self._formatter.register_conversion('n', lambda d: d.name)

        self._terminal = terminal
        self._terminal.reset()

    def _command(self, cmd, *args, **kwargs):
        cmdline = self._formatter.vformat(cmd, args, kwargs)

        return self._terminal.command(cmdline)

    def _command_no_wait(self, cmd, *args, **kwargs):
        cmdline = self._formatter.vformat(cmd, args, kwargs)

        return self._terminal.command_no_wait(cmdline)

    def wait_to_start(self):
        response = self._terminal.command("getState")
        while response != "1":
            time.sleep(0.2)
            response = self._terminal.command("getState")

        self.log.info("OBC reported ready state")

    def reset(self):
        self._terminal.reset()

    def close(self):
        self._terminal.close()

    def power_off(self):
        self._terminal.power_off()

    def power_on(self, clean_state=False):
        self._terminal.power_on(clean_state)

    def ping(self):
        return self._command("ping")
