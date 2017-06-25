from telecommand.base import Telecommand


class EnterIdleState(Telecommand):
    def __init__(self, correlation_id, duration):
        self._correlation_id = correlation_id
        self._duration = duration

    def apid(self):
        return 0xAD

    def payload(self):
        return [self._correlation_id, self._duration]

class SendBeacon(Telecommand):
    def __init__(self):
        pass

    def apid(self):
        return 0xAE

    def payload(self):
        return []
