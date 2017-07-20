from .obc_mixin import OBCMixin, command, decode_return


class SunSMixin(OBCMixin):
    def __init__(self):
        pass

    def _parse_to_array(result):
        return map(int, filter(None, result.split(' ')))

    @decode_return(_parse_to_array)
    @command("suns measure {0} {1}")
    def measure_suns(self):
         pass
