import typing

from .obc_mixin import OBCMixin, command, decode_return, decode_csv


class ErrorCounter:
    def __init__(self, row):
        self.device = int(row['Device'])
        self.current = int(row['Current'])
        self.limit = int(row['Limit'])
        self.increment = int(row['Inc'])
        self.decrement = int(row['Dec'])


def _parse_error_counters(s):
    return map(ErrorCounter, s)


class ErrorCountersMixin(OBCMixin):
    @decode_return(_parse_error_counters)
    @decode_return(decode_csv)
    @command("error_counters current")
    def error_counters(self):
        # type: () -> typing.Dict[int, ErrorCounter]
        pass
