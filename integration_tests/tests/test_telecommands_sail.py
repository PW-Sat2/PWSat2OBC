from response_frames.common import SailSuccessFrame
from system import auto_power_on, clear_state
from telecommand import OpenSailTelecommand
from tests.base import RestartPerTest
from utils import TestEvent


class TestSailTelecommands(RestartPerTest):
    @auto_power_on(auto_power_on=False)
    def __init__(self, *args, **kwargs):
        super(TestSailTelecommands, self).__init__(*args, **kwargs)

    def _start(self):
        e = TestEvent()

        def on_reset(_):
            e.set()

        self.system.comm.on_hardware_reset = on_reset

        self.power_on_obc()

        e.wait_for_change(1)

    @clear_state()
    def test_should_trigger_sail_opening(self):
        self._start()

        sail_opening = TestEvent()

        self.system.eps.TKmain.on_enable = sail_opening.set

        self.system.comm.put_frame(OpenSailTelecommand(0x31))

        ack = self.system.comm.get_frame(5)
        self.assertIsInstance(ack, SailSuccessFrame)

        self.assertTrue(sail_opening.wait_for_change(20), "Sail opening procedure should start")
