from telecommand import Telecommand


class StopAntennaDeployment(Telecommand):
    def __init__(self, correlation_id):
        Telecommand.__init__(self)
        self._correlation_id = correlation_id

    def apid(self):
        return 0xA3

    def payload(self):
        return [self._correlation_id]
