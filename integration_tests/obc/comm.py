import re
from enum import Enum, unique
from devices import TransmitterTelemetry, ReceiverTelemetry
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

    @command("comm send_frame {0}")
    def send_frame(self, data):
        pass

    @decode_return(int)
    @command("com get frame_count")
    def get_frame_count(self):
        pass

    @command("comm receive_frame")
    def receive_frame(self):
        pass

    @command("comm reset {0}")
    def comm_reset(self, module):
        pass

    @command("comm set bitrate {0}")
    def comm_set_bitrate(self, bitrate):
        pass

    def comm_auto_handling(self, enable):
        if not enable:
            self._command("comm pause")

    @staticmethod
    def extract_value(string):
        m = re.search('(.+):\ *\'(\d+)\'', string)
        return (m.group(1), int(m.group(2)))

    @staticmethod
    def set_attribute(object, tupple):
        setattr(object, tupple[0], tupple[1])

    def _parse_transmitter_telemetry(result):
        telemetry = TransmitterTelemetry()
        parts = result.split("\n")
        for part in parts:
            CommMixin.set_attribute(telemetry, CommMixin.extract_value(part))

        return telemetry

    def _parse_receiver_telemetry(result):
        telemetry = ReceiverTelemetry()
        parts = result.split("\n")
        for part in parts:
            CommMixin.set_attribute(telemetry, CommMixin.extract_value(part))

        return telemetry

    @decode_return(_parse_transmitter_telemetry)
    @command("comm get telemetry transmitter")
    def comm_get_transmitter_telemetry(self):
        pass

    @decode_return(_parse_receiver_telemetry)
    @command("comm get telemetry receiver")
    def comm_get_receiver_telemetry(self):
        pass

    @command("comm set idle_state {0:d}")
    def comm_set_idle_state(self, enabled):
        pass
