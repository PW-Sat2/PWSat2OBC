from .obc_mixin import OBCMixin, command, decode_return


class GyroMixin(OBCMixin):
    def __init__(self):
        pass

    def _parse_gyro_result(result):
        parts = map(int, result.split(" "))
        return [int(parts[0]), int(parts[1]), int(parts[2]), int(parts[3])]

    @decode_return(_parse_gyro_result)
    @command("gyro read")
    def gyro_read(self):
        pass

    @command("gyro init")
    def gyro_init(self):
        pass
