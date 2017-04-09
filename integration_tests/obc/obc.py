from datetime import datetime
import logging
from base64 import b64encode

from utils import ExtendableFormatter
from .antenna import AntennaMixin
from .comm import CommMixin
from .experiments import ExperimentsMixin
from .file_system import FileSystemMixin
from .i2c import I2CMixin
from .imtq import ImtqMixin
from .mission import MissionMixin
from .obc_mixin import OBCMixin
from .obc_time import TimeMixin
from .state import StateMixin

class OBC(OBCMixin,
          FileSystemMixin,
          CommMixin,
          TimeMixin,
          I2CMixin,
          AntennaMixin,
          MissionMixin,
          ImtqMixin,
          ExperimentsMixin,
          StateMixin
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
        self.log.debug("Waiting for OBC initialization finish")

        start = datetime.now()

        self._command("wait_for_init")

        end = datetime.now()
        duration = end - start
        self.log.info("OBC initialization done in %s", str(duration))

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
