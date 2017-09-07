from response_frames.compile_info import CompileInfoFrame
from system import auto_power_on
from telecommand import GetCompileInfoTelecommand
from tests.base import RestartPerTest
from utils import TestEvent


class TestGetCompileInfoTelecommand(RestartPerTest):
    @auto_power_on(auto_power_on=False)
    def __init__(self, *args, **kwargs):
        super(TestGetCompileInfoTelecommand, self).__init__(*args, **kwargs)

    def _start(self):
        e = TestEvent()

        def on_reset(_):
            e.set()

        self.system.comm.on_hardware_reset = on_reset

        self.power_on_obc()

        e.wait_for_change(1)

    def test_get_compile_info(self):
        self._start()

        self.system.comm.put_frame(GetCompileInfoTelecommand())

        response = self.system.comm.get_frame(5)
        self.assertIsInstance(response, CompileInfoFrame)
        self.assertEqual(response.seq(), 0)


