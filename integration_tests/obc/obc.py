import logging
from base64 import b64encode

from utils import ExtendableFormatter
from .obc_mixin import OBCMixin
from .file_system import FileSystemMixin
from .antenna import AntennaMixin
from .comm import CommMixin
from .time import TimeMixin
from .i2c import I2CMixin

class OBC(OBCMixin,
          FileSystemMixin,
          CommMixin,
          TimeMixin,
          I2CMixin,
          AntennaMixin
          ):
    def __init__(self, terminal):
        self.log = logging.getLogger("OBC")

        self._formatter = ExtendableFormatter()
        self._formatter.register_conversion('E', lambda d: b64encode(d).rstrip('='))

        self._terminal = terminal
        self._terminal.reset()

    def _command(self, cmd, *args, **kwargs):
        cmdline = self._formatter.vformat(cmd, args, kwargs)

        return self._terminal.command(cmdline)

    def wait_to_start(self):
        response = self._terminal.command("getState")
        while response != "1":
            response = self._terminal.command("getState")

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
