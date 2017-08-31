import telecommand
from threading import Timer

from system import auto_power_on, runlevel
from tests.base import RestartPerTest
from utils import TestEvent


class SunsTelecommandsTest(RestartPerTest):
    @auto_power_on(auto_power_on=False)
    def __init__(self, *args, **kwargs):
        super(SunsTelecommandsTest, self).__init__(*args, **kwargs)

    def _start(self):
        e = TestEvent()

        def on_reset(_):
            e.set()
        self.system.comm.on_hardware_reset = on_reset

        self.power_on_obc()

        e.wait_for_change(1)

    @runlevel(2)
    def test_get_suns_data_sets(self):

        self._start()

        self.system.comm.put_frame(telecommand.GetSunSDataSets(correlation_id=0x11, gain=0x01, itime=0x02))

        frame = self.system.comm.get_frame(20)

        self.assertEqual(frame.apid(), 2)
        self.assertEqual(frame.seq(), 0)
        self.assertEqual(len(frame.payload()), 94)
