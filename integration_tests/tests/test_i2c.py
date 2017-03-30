from time import sleep
from unittest import skip

from devices import EchoDevice, TimeoutDevice
from system import auto_comm_handling, require_two_i2c_buses
from tests.base import BaseTest


class I2CTest(BaseTest):
    def setUp(self):
        BaseTest.setUp(self)

        self.echo = EchoDevice(0x12)
        self.timeoutDevice = TimeoutDevice(0x14)

        self.system.sys_bus.add_bus_device(self.echo)
        self.system.sys_bus.add_pld_device(self.echo)
        self.system.sys_bus.enable_bus_devices([self.echo.address], True)

        self.system.sys_bus.add_bus_device(self.timeoutDevice)
        self.system.sys_bus.add_pld_device(self.timeoutDevice)
        self.system.sys_bus.enable_pld_devices([self.echo.address], True)

    def test_single_transfer(self):
        in_data = '\xb0' * 50
        out_data = ''.join([chr(ord(c) + 1) for c in in_data])

        response = self.system.obc.i2c_transfer('wr', 'system', 0x12, in_data)

        self.assertEqual(response, out_data)

        in_data = 'b'
        out_data = ''.join([chr(ord(c) + 1) for c in in_data])

        response = self.system.obc.i2c_transfer('wr', 'system', 0x12, in_data)

        self.assertEqual(response, out_data)

    def test_single_transfer_both_buses(self):
        in_data = '\xb0' * 50
        out_data = ''.join([chr(ord(c) + 1) for c in in_data])

        response = self.system.obc.i2c_transfer('wr', 'system', 0x12, in_data)

        self.assertEqual(response, out_data)

        in_data = 'b'
        out_data = ''.join([chr(ord(c) + 1) for c in in_data])

        response = self.system.obc.i2c_transfer('wr', 'system', 0x12, in_data)

        self.assertEqual(response, out_data)

        in_data = '\xb0' * 50
        out_data = ''.join([chr(ord(c) + 1) for c in in_data])

        response = self.system.obc.i2c_transfer('wr', 'payload', 0x12, in_data)

        self.assertEqual(response, out_data)

        in_data = 'b'
        out_data = ''.join([chr(ord(c) + 1) for c in in_data])

        response = self.system.obc.i2c_transfer('wr', 'payload', 0x12, in_data)

        self.assertEqual(response, out_data)

    def test_single_transfer_payload(self):
        in_data = 'abc'
        out_data = ''.join([chr(ord(c) + 1) for c in in_data])

        response = self.system.obc.i2c_transfer('wr', 'payload', 0x12, in_data)

        self.assertEqual(response, out_data)

    def test_transfer_on_both_buses(self):
        response = self.system.obc.i2c_transfer('wr', 'system', 0x12, 'abc')

        self.assertEqual(response, 'bcd')

        response = self.system.obc.i2c_transfer('wr', 'payload', 0x12, 'def')

        self.assertEqual(response, 'efg')

    @auto_comm_handling(False)
    @skip('no support for latching')
    def test_should_be_able_to_transfer_on_unlatched_bis(self):
        response = self.system.obc.i2c_transfer('wr', 'system', 0x14, chr(0x02))
        self.assertEqual(response, 'Error -7')

        self.system.sys_bus.unlatch()
        self.system.sys_bus.unfreeze()
        sleep(0.1)

        response = self.system.obc.i2c_transfer('wr', 'system', 0x12, 'abc')
        self.assertEqual(response, 'bcd')

    def test_should_detect_nak_on_buses(self):
        self.system.sys_bus.disable_bus()
        self.system.sys_bus.disable_payload()

        response = self.system.obc.i2c_transfer('wr', 'system', 0x12, 'abc')

        self.assertEqual(response, 'Error -1')

        response = self.system.obc.i2c_transfer('wr', 'payload', 0x12, 'abc')

        self.assertEqual(response, 'Error -1')

    @require_two_i2c_buses
    @skip('no support for latching')
    def test_bus_latch_should_trigger_system_power_cycle(self):
        self.system.obc.i2c_transfer('wr', 'system', 0x14, chr(0x2))

        self.assertTrue(self.system.eps.power_cycle.wait(5), "Power cycle should be triggered")

        self.system.restart()

        response = self.system.obc.i2c_transfer('wr', 'system', 0x12, 'abc')

        self.assertEqual(response, 'bcd')

    @auto_comm_handling(False)
    # @skip('requires DeviceMock v3')
    def test_isis_behaviour(self):
        echo2 = EchoDevice(0x16)
        self.system.sys_bus.add_bus_device(echo2)
        self.system.sys_bus.enable_bus_devices([echo2.address], True)

        self.system.obc.i2c_transfer('w', 'system', 0x12, 'abc')

        self.system.obc.i2c_transfer('w', 'system', 0x16, 'def')

        echo1_response = self.system.obc.i2c_transfer('r', 'system', 0x12, '3')

        echo2_response = self.system.obc.i2c_transfer('r', 'system', 0x16, '3')

        self.assertEqual(echo1_response, 'bcd')
        self.assertEqual(echo2_response, 'efg')
