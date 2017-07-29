from telecommand import Telecommand
from utils import ensure_byte_list


class SetPeriodicMessageTelecommand(Telecommand):
    def __init__(self, correlation_id, interval_minutes, repeat_count, message):
        Telecommand.__init__(self)
        self._message = message
        self._repeat_count = repeat_count
        self._interval_minutes = interval_minutes
        self._correlation_id = correlation_id

    def apid(self):
        return 0x05

    def payload(self):
        return [self._correlation_id, self._interval_minutes, self._repeat_count] + ensure_byte_list(self._message)
