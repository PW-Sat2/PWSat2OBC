from response_frames.operation import OperationSuccessFrame
from system import runlevel, auto_power_on
from telecommand import SetBootSlots
from tests.base import BaseTest
from utils import TestEvent


class BootSettingsTelecomandsTest(BaseTest):
    @auto_power_on(auto_power_on=False)
    def __init__(self, *args, **kwargs):
        super(BootSettingsTelecomandsTest, self).__init__(*args, **kwargs)

    def _start(self):
        e = TestEvent()

        def on_reset(_):
            e.set()
        self.system.comm.on_hardware_reset = on_reset

        self.power_on_obc(clean_state=True)

        e.wait_for_change(1)

    @runlevel(2)
    def test_x(self):
        self._start()
        self.system.comm.put_frame(SetBootSlots(correlation_id=0x14, primary=0b111, failsafe=0b111000))

        f = self.system.comm.get_frame(10)
        self.assertIsInstance(f, OperationSuccessFrame)
        self.assertEqual(f.response, [0b111, 0b111000])

        b = self.system.obc.boot_settings()

        self.assertEqual(b['Primary'], 0b111)
        self.assertEqual(b['Failsafe'], 0b111000)
