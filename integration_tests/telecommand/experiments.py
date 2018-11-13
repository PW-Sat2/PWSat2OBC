import struct

from utils import ensure_byte_list
from telecommand.base import CorrelatedTelecommand


class PerformDetumblingExperiment(CorrelatedTelecommand):
    def apid(self):
        return 0x0D

    def payload(self):
        return struct.pack('<BLB', self._correlation_id, self._duration.total_seconds(), self.sampling_interval.total_seconds())

    def __init__(self, correlation_id, duration, sampling_interval):
        super(PerformDetumblingExperiment, self).__init__(correlation_id)
        self.sampling_interval = sampling_interval
        self._duration = duration


class AbortExperiment(CorrelatedTelecommand):
    def __init__(self, correlation_id):
        super(AbortExperiment, self).__init__(correlation_id)

    def apid(self):
        return 0x0E

    def payload(self):
        return [self._correlation_id]


class PerformSunSExperiment(CorrelatedTelecommand):
    def apid(self):
        return 0x1D

    def payload(self):
        return struct.pack('<BBBBBBB',
                           self._correlation_id,
                           self.gain,
                           self.itime,
                           self.samples_count,
                           int(self.short_delay.total_seconds()),
                           self.sessions_count,
                           int(self.long_delay.total_seconds() / 60.0)
                           ) + self.file_name + '\0'

    def __init__(self, correlation_id, gain, itime, samples_count, short_delay, sessions_count, long_delay, file_name):
        CorrelatedTelecommand.__init__(self, correlation_id)
        self.long_delay = long_delay
        self.sessions_count = sessions_count
        self.short_delay = short_delay
        self.samples_count = samples_count
        self.itime = itime
        self.gain = gain
        self.file_name = file_name


class PerformRadFETExperiment(CorrelatedTelecommand):
    def apid(self):
        return 0x1E

    def payload(self):
        return struct.pack('<BBB', self._correlation_id, self.delay, self.samples_count) + self.output_file_name + '\0'

    def __init__(self, correlation_id, delay, samples_count, output_file_name):
        CorrelatedTelecommand.__init__(self, correlation_id)
        self.delay = delay
        self.samples_count = samples_count
        self.output_file_name = output_file_name


class PerformSailExperiment(CorrelatedTelecommand):
    def __init__(self, correlation_id):
        CorrelatedTelecommand.__init__(self, correlation_id)

    def apid(self):
        return 0x10

    def payload(self):
        return [self._correlation_id]


class PerformPayloadCommissioningExperiment(CorrelatedTelecommand):
    def apid(self):
        return 0x20

    def payload(self):
        return struct.pack('<B', self._correlation_id) + self.file_name + '\0'

    def __init__(self, correlation_id, file_name):
        CorrelatedTelecommand.__init__(self, correlation_id)
        self.file_name = file_name


class PerformSADSExperiment(CorrelatedTelecommand):
    def __init__(self, correlation_id):
        super(PerformSADSExperiment, self).__init__(correlation_id)

    def apid(self):
        return 0xD4

    def payload(self):
        return [self._correlation_id]


class PerformCameraCommissioningExperiment(CorrelatedTelecommand):
    def apid(self):
        return 0x26

    def payload(self):
        return struct.pack('<B', self._correlation_id) + self.file_name + '\0'

    def __init__(self, correlation_id, file_name):
        super(PerformCameraCommissioningExperiment, self).__init__(correlation_id)
        self.file_name = file_name


class CopyBootSlots(CorrelatedTelecommand):
    def apid(self):
        return 0x28

    def payload(self):
        return [self._correlation_id, self.source_mask, self.target_mask]

    def __init__(self, correlation_id, source_mask, target_mask):
        super(CopyBootSlots, self).__init__(correlation_id)
        self.target_mask = target_mask
        self.source_mask = source_mask
