from telecommand import Telecommand


class OpenSailTelecommand(Telecommand):
    def __init__(self, correlation_id):
        Telecommand.__init__(self)
        self._correlation_id = correlation_id

    def apid(self):
        return 0x04

    def payload(self):
        return [self._correlation_id]