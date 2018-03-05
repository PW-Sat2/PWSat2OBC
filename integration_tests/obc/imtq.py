from .obc_mixin import OBCMixin, command, decode_return


class ImtqMixin(OBCMixin):
    def __init__(self):
        pass

    def _parse_mgtm(result):
        parts = map(int, result.split(" "))
        return [int(parts[0]), int(parts[1]), int(parts[2])]

    def _parse_to_array(result):
        return map(int, result.split(' '))

    def _parse_lines_to_array(result):
        return [map(int, line.split(' ')) for line in result.splitlines()]

    @decode_return(_parse_mgtm)
    @command("imtq mtmRead")
    def read_magnetometer(self):
         pass

    @command("imtq bdot {0}")
    def bdot(self):
        pass

    @decode_return(_parse_to_array)
    @command("imtq detumbleGet {0}")
    def get_detumble_data(self, fix=0):
        pass

    @decode_return(_parse_lines_to_array)
    @command("imtq PerformSelfTest {0}")
    def perform_self_test(self, fix=0):
        pass
