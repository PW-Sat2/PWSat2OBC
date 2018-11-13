from telecommand.base import Telecommand, CorrelatedTelecommand


class PowerCycleTelecommand(CorrelatedTelecommand):
    def __init__(self, correlation_id):
        super(PowerCycleTelecommand, self).__init__(correlation_id)

    def apid(self):
        return 0x01

    def payload(self):
        return [self._correlation_id]
