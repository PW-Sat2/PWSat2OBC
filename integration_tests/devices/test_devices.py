import i2cMock


class EchoDevice(i2cMock.I2CDevice):
    @i2cMock.command([])
    def _echo(self, *args):
        return [c + 1 for c in args]


class TimeoutDevice(i2cMock.I2CDevice):
    @i2cMock.command([0x02])
    def _freeze(self, *args):
        self.latch()
        return list(args)
