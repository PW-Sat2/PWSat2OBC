from telecommand import Telecommand
from typing import List, Tuple
import struct


class ErrorCounterConfiguration:
    def __init__(self, device, limit, increment, decrement):
        self.device = device
        self.limit = limit
        self.increment = increment
        self.decrement = decrement

    def pack(self):
        return struct.pack('<BBBB',
                           self.device,
                           self.limit,
                           self.increment,
                           self.decrement)


class SetErrorCounterConfig(Telecommand):
    def __init__(self, correlation_id, configs):
        # type: (int, List[ErrorCounterConfiguration]) -> Any
        Telecommand.__init__(self)

        self._configs = configs
        self._correlation_id = correlation_id

    def apid(self):
        return 0x02

    def payload(self):
        result = [self._correlation_id]
        for c in self._configs:
            result += list(c.pack())

        return result


class GetErrorCounterConfig(Telecommand):
    def __init__(self):
        Telecommand.__init__(self)
        
    def apid(self):
        return 0x06

    def payload(self):
        return []
