import struct
from datetime import timedelta

import emulator.beacon_parser.eps_controller_a_telemetry_parser as epsa
import emulator.beacon_parser.eps_controller_b_telemetry_parser as epsb

from common import GenericSuccessResponseFrame, DownlinkApid
from response_frames import response_frame, ResponseFrame
from utils import ensure_string


@response_frame(DownlinkApid.LittleOryxReboot)
class RebootSuccessFrame(GenericSuccessResponseFrame):
    pass


@response_frame(DownlinkApid.LittleOryxDelayReboot)
class DelayRebootSuccessFrame(GenericSuccessResponseFrame):
    pass


@response_frame(DownlinkApid.LittleOryxEcho)
class EchoFrame(ResponseFrame):
    @classmethod
    def matches(cls, payload):
        return True

    def decode(self):
        self.message = self.payload()

    def __str__(self):
        return 'Echo({})'.format(''.join(map(chr, self.message)))

    def __repr__(self):
        return str(self)


@response_frame(DownlinkApid.LittleOryxDeepSleepBeacon)
class LittleOryxDeepSleepBeacon(ResponseFrame):

    @classmethod
    def matches(cls, payload):
        return True

    def decode(self):
        (
            uptime, vbat_a, vbat_b, reboot_counter, distr_3v3_current, distr_5v_current, distr_vbat_current, temp_a,
            temp_b,
            text) = struct.unpack(
            '<LHHLHHHhh27s', ensure_string(self.payload()))
        self.text = text
        self.uptime = timedelta(milliseconds=uptime)
        self.reboot_counter = reboot_counter
        self.distr_3v3_current = epsa.DistributionCurrent(distr_3v3_current)
        self.distr_5v_current = epsa.DistributionCurrent(distr_5v_current)
        self.distr_vbat_current = epsa.DistributionCurrent(distr_vbat_current)
        self.vbat_a = epsa.BATCVoltage(vbat_a)
        self.temp_a = epsa.TMP121Temperature(temp_a)
        self.temp_b = epsa.TMP121Temperature(temp_b)
        self.vbat_b = epsb.BATCVoltage(vbat_b)

    def __str__(self):
        lines = [
            '\tTime: {}s',
            '\tVBAT_A: {}',
            '\tVBAT_B: {}',
            '\tReboot to normal: {}',
            '\t3V3 Current: {:.0f} mA',
            '\t5V Current: {:.0f} mA',
            '\tVbat Current: {:.0f} mA',
            '\tTemp A: {}',
            '\tTemp B: {}'
        ]
        data = [
            self.uptime.seconds, #
            self.vbat_a, 
            self.vbat_b, 
            self.reboot_counter, 
            self.distr_3v3_current.converted * 1000.0,
            self.distr_5v_current.converted * 1000.0, 
            self.distr_vbat_current.converted * 1000.0, 
            self.temp_a, 
            self.temp_b
        ]
        return '\n'.join(lines).format(*data)

    def __repr__(self):
        return str(self)
