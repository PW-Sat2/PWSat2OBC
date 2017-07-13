import telecommand
from response_frames.operation import OperationSuccessFrame
from devices import EchoDevice, BeaconFrame
from system import auto_power_on, runlevel
from tests.base import BaseTest, RestartPerTest
from utils import ensure_byte_list, TestEvent

class RawI2CTelecommandsTest(RestartPerTest):
    @auto_power_on(auto_power_on=False)
    def __init__(self, *args, **kwargs):
        super(RawI2CTelecommandsTest, self).__init__(*args, **kwargs)

    def _start(self):
        e = TestEvent()

        def on_reset(_):
            e.set()
        self.system.comm.on_hardware_reset = on_reset

        self.power_on_obc()

        e.wait_for_change(1)
        
        self.systemEcho = EchoDevice(0x12, "Echo")
        self.payloadEcho = EchoDevice(0x13, "Echo")

        self.system.i2c.add_bus_device(self.systemEcho)
        self.system.i2c.add_pld_device(self.payloadEcho)
        self.system.i2c.enable_bus_devices([self.systemEcho.address], True)
        self.system.i2c.enable_pld_devices([self.payloadEcho.address], True)
        

    @runlevel(2)
    def test_send_raw_system_bus_data(self):
        self._start()

        self.system.comm.put_frame(telecommand.RawI2C(correlation_id=0x11, busSelect=0, address=self.systemEcho.address, delay=100, data=[32, 64, 50, 104]))

        frame = self.system.comm.get_frame(20)

        self.assertEqual(frame.seq(), 0)
        self.assertEqual(frame.correlation_id, 0x11)
        self.assertEqual(frame.payload(), [17, 0, 33, 65, 51, 105] + [0]*226)


    @runlevel(2)
    def test_send_raw_payload_data(self):
        self._start()

        self.system.comm.put_frame(telecommand.RawI2C(correlation_id=0x11, busSelect=1, address=self.payloadEcho.address, delay=100, data=[1, 2, 3, 4, 5]))

        frame = self.system.comm.get_frame(20)

        self.assertEqual(frame.seq(), 0)
        self.assertEqual(frame.correlation_id, 0x11)
        self.assertEqual(frame.payload(), [17, 0, 2, 3, 4, 5, 6] + [0]*225)

        
    @runlevel(2)
    def test_send_data_without_delay(self):
        self._start()

        self.system.comm.put_frame(telecommand.RawI2C(correlation_id=0x11, busSelect=0, address=self.systemEcho.address, delay=0, data=[6, 66, 6]))

        frame = self.system.comm.get_frame(20)

        self.assertEqual(frame.seq(), 0)
        self.assertEqual(frame.correlation_id, 0x11)
        self.assertEqual(frame.payload(), [17, 0, 7, 67, 7] + [0]*227)
        