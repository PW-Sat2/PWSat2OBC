import telecommand

from system import auto_power_on
from tests.base import BaseTest
from utils import ensure_byte_list, TestEvent


class TimeTelecommandsTest(BaseTest):
    @auto_power_on(auto_power_on=False)
    def __init__(self, *args, **kwargs):
        super(TimeTelecommandsTest, self).__init__(*args, **kwargs)

    def _start(self):
        e = TestEvent()

        def on_reset(_):
            e.set()
        self.system.comm.on_hardware_reset = on_reset

        self.system.obc.power_on(clean_state=True)
        self.system.obc.wait_to_start()

        e.wait_for_change(1)

    def test_set_time_correction_config(self):
        self._start()

        self.system.comm.put_frame(telecommand.SetTimeCorrectionConfig(correlation_id=0x11, missionTimeWeight=0x12, externalTimeWeight=0x34))

        frame = self.system.comm.get_frame(20)

        time_config_text = self.system.obc.state_get_time_config()
        time_config = time_config_text.split()
        del time_config[-1]
        
        self.assertEqual(frame.apid(), 2)
        self.assertEqual(frame.seq(), 0)
        self.assertEqual(frame.payload(), [0x11, 0])
        self.assertEqual(map(int, time_config), [0x12, 0x34])
        