import logging

from .CommandFormatter import CommandFormatter
from .obc_mixin import OBCMixin
from .file_system import FileSystemMixin
from .comm import CommMixin
from .time import TimeMixin
from .i2c import I2CMixin


class OBC(OBCMixin,
          FileSystemMixin
):
    def __init__(self, terminal):
        self.log = logging.getLogger("OBC")

        self._formatter = CommandFormatter()

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

    def i2c_transfer(self, mode, bus, address, data):
        return self._terminal.command("i2c %s %s %d %s" % (mode, bus, address, data))

    def ping(self):
        return self._terminal.command("ping")

    def jump_to_time(self, time):
        self._terminal.command("jumpToTime %d" % time)

    def current_time(self):
        r = self._terminal.command("currentTime")
        return int(r)

    def send_frame(self, data):
        self._terminal.command("sendFrame %s" % data)

    def get_frame_count(self):
        r = self._terminal.command("getFramesCount")
        return int(r)

    def receive_frame(self):
        r = self._terminal.command("receiveFrame")
        return r

    def comm_auto_handling(self, enable):
        if not enable:
            self._terminal.command("pauseComm")
