import i2cMock
from tests.base import BaseTest


class EchoDevice(i2cMock.I2CDevice):
    @i2cMock.command([])
    def _echo(self, *args):
        return list(args)


class I2CTest(BaseTest):
    def setUp(self):
        BaseTest.setUp(self)

        self.echo = EchoDevice(0x12)

        self.system.i2c.add_device(self.echo)
        self.system.payload.add_device(self.echo)

    def test_transfer_on_both_buses(self):
        response = self.system.obc.i2c_transfer('system', 0x12, 'abc')

        self.assertEqual(response, 'abc')

        response = self.system.obc.i2c_transfer('payload', 0x12, 'def')

        self.assertEqual(response, 'def')
