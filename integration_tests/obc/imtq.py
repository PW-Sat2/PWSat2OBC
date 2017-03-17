from .obc_mixin import OBCMixin, command, decode_return


class ImtqMixin(OBCMixin):
    def __init__(self):
        pass

    def _parse_mgtm(result):
        parts = map(int, result.split(" "))
        return [int(parts[0]), int(parts[1]), int(parts[2])]

    @decode_return(_parse_mgtm)
    @command("imtq mtmRead")
    def read_magnetometer(self):
         pass

    @command("imtq bdot {0}")
    def bdot(self):
        pass
