from telecommand.base import Telecommand


class DisableOverheatSubmode(Telecommand):
    def __init__(self, correlation_id, controller):
        self._correlation_id = correlation_id
        self._controller = controller

    def apid(self):
        return 0x19

    def payload(self):
        return [self._correlation_id, self._controller]
