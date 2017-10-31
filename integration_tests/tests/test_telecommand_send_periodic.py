from response_frames.period_message import PeriodicMessageFrame
from system import runlevel, auto_power_on
from telecommand import SendPeriodicMessageTelecommand
from tests.base import BaseTest, RestartPerTest
from utils import TestEvent


@runlevel(2)
class SendPeriodicMessageTelecommandTest(RestartPerTest):
    @auto_power_on(auto_power_on=False)
    def __init__(self, *args, **kwargs):
        super(SendPeriodicMessageTelecommandTest, self).__init__(*args, **kwargs)

    def _start(self):
        e = TestEvent()

        def on_reset(_):
            e.set()

        self.system.comm.on_hardware_reset = on_reset

        self.power_on_obc()

        e.wait_for_change(1)

    def test_send_periodic_message(self):
        self._start()

        self.system.comm.put_frame(SendPeriodicMessageTelecommand(5))

        for i in xrange(0, 5):
            f = self.system.comm.get_frame(10, filter_type=PeriodicMessageFrame)

            self.assertIsInstance(f, PeriodicMessageFrame)