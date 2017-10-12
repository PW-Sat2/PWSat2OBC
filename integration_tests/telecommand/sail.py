from telecommand import Telecommand


class OpenSailTelecommand(Telecommand):
    def __init__(self, correlation_id, ignore_overheat):
        Telecommand.__init__(self)
        self.correlation_id = correlation_id
        self.ignore_overheat = ignore_overheat

    def apid(self):
        return 0x04

    def payload(self):
        return [self.correlation_id, int(self.ignore_overheat)]


class StopSailDeployment(Telecommand):
    def __init__(self, correlation_id):
        Telecommand.__init__(self)
        self.correlation_id = correlation_id

    def apid(self):
        return 0x2D

    def payload(self):
        return [self.correlation_id]
