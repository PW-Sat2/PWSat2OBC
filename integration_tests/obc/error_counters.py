from .obc_mixin import OBCMixin, command, decode_return


def _parse_error_counters(s):
    counts = map(int, s.strip().split(' '))
    return dict(zip(range(0, len(counts)), counts))


class ErrorCountersMixin(OBCMixin):
    @decode_return(_parse_error_counters)
    @command("error_counters")
    def error_counters(self):
        pass
