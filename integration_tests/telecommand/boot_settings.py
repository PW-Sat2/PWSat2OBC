from telecommand.base import Telecommand, CorrelatedTelecommand


class SetBootSlots(CorrelatedTelecommand):
    def __init__(self, correlation_id, primary, failsafe):
        super(SetBootSlots, self).__init__(correlation_id)
        self._primary = primary
        self._failsafe = failsafe

    def apid(self):
        return 0x03

    def payload(self):
        return [self._correlation_id, self._primary, self._failsafe]

