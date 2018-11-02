from telecommand.base import Telecommand, CorrelatedTelecommand


class OpenSailTelecommand(CorrelatedTelecommand):
    def __init__(self, correlation_id, ignore_overheat):
        super(OpenSailTelecommand, self).__init__(correlation_id)
        self.ignore_overheat = ignore_overheat

    def apid(self):
        return 0x04

    def payload(self):
        return [self._correlation_id, int(self.ignore_overheat)]


class StopSailDeployment(CorrelatedTelecommand):
    def __init__(self, correlation_id):
        super(StopSailDeployment, self).__init__(correlation_id)

    def apid(self):
        return 0x2D

    def payload(self):
        return [self._correlation_id]
