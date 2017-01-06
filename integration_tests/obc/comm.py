from enum import Enum, unique
from .obc_mixin import OBCMixin, command, decode_return

@unique
class CommModule(Enum):
    Both = 0
    Transmitter = 1
    Receiver = 2
    Watchdog = 3

    def __str__(self):
        map = {
            self.Both: "hardware",
            self.Transmitter: "transmitter",
            self.Receiver: "receiver",
            self.Watchdog: "watchdog",
        }

        return map[self]

class CommMixin(OBCMixin):
    def __init__(self):
        pass

    @command("sendFrame {0}")
    def send_frame(self, data):
        pass

    @decode_return(int)
    @command("getFramesCount")
    def get_frame_count(self):
        pass

    @command("receiveFrame")
    def receive_frame(self):
        pass

    @command("comm_reset {0}")
    def comm_reset(self, module):
        pass

    def comm_auto_handling(self, enable):
        if not enable:
            self._command("pauseComm")
