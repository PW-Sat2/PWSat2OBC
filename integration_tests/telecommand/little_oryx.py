import struct

from telecommand.base import CorrelatedTelecommand, Telecommand


class Reboot(Telecommand):
    def apid(self):
        return 0xC0

    def payload(self):
        return ''


class RebootToNormal(Telecommand):
    def apid(self):
        return 0xC2

    def payload(self):
        return ''


class DelayRebootToNormal(CorrelatedTelecommand):
    def __init__(self, correlation_id, reboot_count):
        super(DelayRebootToNormal, self).__init__(correlation_id)
        self.reboot_count = reboot_count

    def apid(self):
        return 0xC1

    def payload(self):
        return struct.pack('<BB', self.correlation_id(), self.reboot_count)


class Echo(Telecommand):
    def __init__(self, message):
        super(Echo, self).__init__()
        self.message = message

    def apid(self):
        return 0xC3

    def payload(self):
        return self.message
