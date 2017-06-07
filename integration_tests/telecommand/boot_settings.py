from telecommand import Telecommand


class SetBootSlots(Telecommand):
    def apid(self):
        return 0x03

    def payload(self):
        return [self._correlation_id, self._primary, self._failsafe]

    def __init__(self, correlation_id, primary, failsafe):
        self._correlation_id = correlation_id
        self._primary = primary
        self._failsafe = failsafe
