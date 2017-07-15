from telecommand import Telecommand
from typing import List, Tuple


class SetErrorCounterConfig(Telecommand):
    def __init__(self, correlation_id, configs):
        # type: (int, List[Tuple[int,int,int,int]]) -> Any
        Telecommand.__init__(self)

        self._configs = configs
        self._correlation_id = correlation_id

    def apid(self):
        return 0x02

    def payload(self):
        result = [self._correlation_id]
        for c in self._configs:
            result += list(c)

        return result
