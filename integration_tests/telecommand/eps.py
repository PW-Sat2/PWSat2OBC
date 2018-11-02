from telecommand.base import Telecommand, CorrelatedTelecommand


class DisableOverheatSubmode(CorrelatedTelecommand):
    def __init__(self, correlation_id, controller):
        super(DisableOverheatSubmode, self).__init__(correlation_id)
        self._controller = controller

    def apid(self):
        return 0x19

    def payload(self):
        return [self._correlation_id, self._controller]
