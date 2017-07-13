from system import auto_power_on
from telecommand import PowerCycleTelecommand
from tests.base import RestartPerTest
from utils import TestEvent
from response_frames.operation import OperationSuccessFrame


class TestPowerTelecommands(RestartPerTest):
    @auto_power_on(auto_power_on=False)
    def __init__(self, *args, **kwargs):
        super(TestPowerTelecommands, self).__init__(*args, **kwargs)

    def _start(self):
        e = TestEvent()

        def on_reset(_):
            e.set()

        self.system.comm.on_hardware_reset = on_reset

        self.power_on_obc()

        e.wait_for_change(1)

    def test_should_trigger_power_cycle(self):
        self._start()

        power_cycle_requested = TestEvent()

        self.system.eps.controller_a.on_power_cycle = power_cycle_requested.set

        self.system.comm.put_frame(PowerCycleTelecommand(0x21))

        ack = self.system.comm.get_frame(5)
        self.assertIsInstance(ack, OperationSuccessFrame)

        power_cycle_requested.wait_for_change(1)

        self.system.restart()