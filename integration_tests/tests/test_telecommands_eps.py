import telecommand
from response_frames.operation import OperationSuccessFrame
from response_frames.disable_overheat_submode import DisableOverheatSubmodeSuccessFrame
from devices import BeaconFrame
from system import auto_power_on, runlevel
from tests.base import BaseTest, RestartPerTest
from utils import ensure_byte_list, TestEvent


class EpsTelecommandsTest(RestartPerTest):
    @auto_power_on(auto_power_on=False)
    def __init__(self, *args, **kwargs):
        super(EpsTelecommandsTest, self).__init__(*args, **kwargs)

    def _start(self):
        event = TestEvent()

        def on_reset(_):
            event.set()
        self.system.comm.on_hardware_reset = on_reset

        self.power_on_obc()

        event.wait_for_change(1)

    @runlevel(2)
    def test_disable_overheat_submode(self):
        self._start()

        event = TestEvent()

        self.system.eps.controller_b.on_disable_overheat_submode = event.set

        self.system.comm.put_frame(telecommand.DisableOverheatSubmode(correlation_id=0x28, controller=1))

        frame = self.system.comm.get_frame(20)
        
        self.assertIsInstance(frame, DisableOverheatSubmodeSuccessFrame)
        self.assertEqual(frame.seq(), 0)
        self.assertEqual(frame.correlation_id, 0x28)
        self.assertTrue(event.wait_for_change(2), "Should disable overheat mode (controller B)")
