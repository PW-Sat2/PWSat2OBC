from response_frames.operation import OperationSuccessFrame
from system import auto_power_on
from telecommand import GetPersistentState
from response_frames.persistent_state import PersistentStateFrame
from tests.base import RestartPerTest
from utils import TestEvent

class TestGetPersistentStateTelecommand(RestartPerTest):
    @auto_power_on(auto_power_on=False)
    def __init__(self, *args, **kwargs):
        super(TestGetPersistentStateTelecommand, self).__init__(*args, **kwargs)

    def _start(self):
        e = TestEvent()

        def on_reset(_):
            e.set()

        self.system.comm.on_hardware_reset = on_reset

        self.power_on_obc()

        e.wait_for_change(1)

    def test_should_trigger_sail_opening(self):
        self._start()

        self.system.comm.put_frame(GetPersistentState())

        response = self.system.comm.get_frame(5)
        self.assertIsInstance(response, PersistentStateFrame)
        self.assertEqual(response.seq(), 0);

        response = self.system.comm.get_frame(1)
        self.assertIsInstance(response, PersistentStateFrame)
        self.assertEqual(response.seq(), 1);
