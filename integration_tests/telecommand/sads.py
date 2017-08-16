from telecommand import Telecommand


class DeploySolarArrayTelecommand(Telecommand):
    def __init__(self, correlation_id):
        Telecommand.__init__(self)
        self._correlation_id = correlation_id

    def apid(self):
        return 0xD4

    def payload(self):
        return [self._correlation_id]