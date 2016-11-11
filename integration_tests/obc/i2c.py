from .obc_mixin import OBCMixin, command


class I2CMixin(OBCMixin):
    def __init__(self):
        pass

    @command("i2c {0} {1} {2} {3}")
    def i2c_transfer(self, mode, bus, address, data):
        pass
