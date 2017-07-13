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
        
        self.echo = EchoDevice(0x12, "Echo")

        self.system.i2c.add_bus_device(self.echo)
        self.system.i2c.enable_bus_devices([self.echo.address], True)
        

    @runlevel(2)
    def test_send_raw_data(self):
        self._start()

        self.system.comm.put_frame(telecommand.RawI2C(correlation_id=0x11, address=self.echo.address, data=[32, 64, 50, 104]))

        frame = self.system.comm.get_frame(20)

        self.assertEqual(frame.seq(), 0)
        self.assertEqual(frame.correlation_id, 0x11)
        self.assertEqual(frame.payload(), [17, 0, 33, 65, 51, 105] + [0]*226)
