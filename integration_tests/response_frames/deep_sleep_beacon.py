
import struct
import emulator.beacon_parser.eps_controller_a_telemetry_parser as epsa
import  emulator.beacon_parser.eps_controller_b_telemetry_parser as epsb
from response_frames import ResponseFrame, response_frame
from utils import ensure_string

@response_frame(0x24)
class DeepSleepBeacon(ResponseFrame):
    @classmethod
    def matches(cls, payload):
        return True

    def decode(self):
        (self._time, self._raw_vbat_a, self._raw_vbat_b) = struct.unpack('<LHH', ensure_string(self.payload()))
        self._vbat_a = epsa.BATCVoltage(self._raw_vbat_a)
        self._vbat_b = epsb.BATCVoltage(self._raw_vbat_b)

    def __str__(self):
        return '\tTime:{} s\n\tVBAT_A:{}\n\tVBAT_B: {}'.format(self._time, self._vbat_a, self._vbat_b)

    def __repr__(self):
        return str(self)