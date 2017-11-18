from datetime import timedelta

import telecommand

from response_frames.common import DownlinkApid
from response_frames.time import TimeCorrectionSuccessFrame, TimeSetSuccessFrame
from system import auto_power_on, runlevel
from tests.base import  RestartPerTest
from utils import TestEvent


class TimeTelecommandsTest(RestartPerTest):
    @auto_power_on(auto_power_on=False)
    def __init__(self, *args, **kwargs):
        super(TimeTelecommandsTest, self).__init__(*args, **kwargs)

    def _start(self):
        e = TestEvent()

        def on_reset(_):
            e.set()
        self.system.comm.on_hardware_reset = on_reset

        self.power_on_obc()

        e.wait_for_change(1)

    @runlevel(2)
    def test_set_time_correction_config(self):
        self._start()

        self.system.comm.put_frame(telecommand.SetTimeCorrectionConfig(correlation_id=0x11, missionTimeWeight=0x12, externalTimeWeight=0x34))

        frame = self.system.comm.get_frame(20, filter_type=TimeCorrectionSuccessFrame)

        time_config_text = self.system.obc.state_get_time_config()
        time_config = time_config_text.split()
        del time_config[-1]
        
        self.assertEqual(frame.apid(), DownlinkApid.TimeCorrection)
        self.assertEqual(frame.seq(), 0)
        self.assertEqual(frame.payload(), [0x11, 0])
        self.assertEqual(map(int, time_config), [0x12, 0x34])

    @runlevel(2)
    def test_set_time(self):
        self._start()

        self.system.obc.jump_to_time(timedelta(minutes=2))

        self.system.comm.put_frame(telecommand.SetTime(correlation_id=0x12, new_time=timedelta(seconds=0x1234)))

        frame = self.system.comm.get_frame(20, filter_type=TimeSetSuccessFrame)

        time_text = self.system.obc.state_get_time_state()
        time = time_text.split()

        obc_time = self.system.obc.current_time()

        self.assertAlmostEqual(obc_time, timedelta(seconds=0x1234), delta=timedelta(seconds=2))

    @runlevel(2)
    def test_set_time_without_rtc(self):
        self._start()

        self.system.obc.jump_to_time(timedelta(minutes=2))

        self.system.i2c.enable_pld_devices([self.system.rtc.address], False)

        self.system.comm.put_frame(telecommand.SetTime(correlation_id=0x12, new_time=timedelta(seconds=0x1234)))

        frame = self.system.comm.get_frame(20, filter_type=TimeSetSuccessFrame)

        time_text = self.system.obc.state_get_time_state()
        time = time_text.split()

        obc_time = self.system.obc.current_time()

        self.assertAlmostEqual(obc_time, timedelta(seconds=0x1234), delta=timedelta(seconds=2))

