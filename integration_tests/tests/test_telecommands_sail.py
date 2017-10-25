from response_frames.common import SailSuccessFrame
from system import auto_power_on, clear_state
from telecommand import OpenSailTelecommand, StopSailDeployment
from response_frames.stop_sail_deployment import StopSailDeploymentSuccessFrame
from tests.base import RestartPerTest
from datetime import timedelta
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
        overheat_disabled = TestEvent()

        self.system.eps.TKmain.on_enable = sail_opening.set
        self.system.eps.controller_a.on_disable_overheat_submode = overheat_disabled.set

        self.system.comm.put_frame(OpenSailTelecommand(0x31, False))

        ack = self.system.comm.get_frame(5, filter_type=SailSuccessFrame)
        self.assertIsInstance(ack, SailSuccessFrame)

        self.assertTrue(sail_opening.wait_for_change(20), "Sail opening procedure should start")
        self.assertFalse(overheat_disabled.wait_for_change(0), "Overheat should not be disabled")

    @clear_state()
    def test_disable_sail_deployment_should_not_disable_deployment_on_command(self):
        self._start()
        self.system.obc.run_mission()

        sail_opening = TestEvent()
        overheat_disabled = TestEvent()

        self.system.eps.TKmain.on_enable = sail_opening.set
        self.system.eps.controller_a.on_disable_overheat_submode = overheat_disabled.set

        self.system.comm.put_frame(StopSailDeployment(11))
        frame = self.system.comm.get_frame(20, filter_type=StopSailDeploymentSuccessFrame)
        self.assertIsInstance(frame, StopSailDeploymentSuccessFrame)

        self.system.obc.run_mission()

        self.system.comm.put_frame(OpenSailTelecommand(0x31, True))
        frame = self.system.comm.get_frame(20, filter_type=SailSuccessFrame)
        self.assertIsInstance(frame, SailSuccessFrame)

        self.system.obc.run_mission()

        self.assertTrue(sail_opening.wait_for_change(5), "Sail deployment should be performed")
        self.assertTrue(overheat_disabled.wait_for_change(1), "Overheat should be disabled")

    @clear_state()
    def test_disable_sail_deployment_should_disable_automatic_deploy(self):
        self._start()
        self.system.obc.run_mission()

        sail_opening = TestEvent()
        self.system.eps.TKmain.on_enable = sail_opening.set

        self.system.comm.put_frame(StopSailDeployment(11))
        frame = self.system.comm.get_frame(20, filter_type=StopSailDeploymentSuccessFrame)
        self.assertIsInstance(frame, StopSailDeploymentSuccessFrame)

        self.system.obc.run_mission()

        self.system.obc.jump_to_time(timedelta(days=41))

        self.system.obc.run_mission()

        self.assertFalse(sail_opening.wait_for_change(5), "Sail deployment should not be performed")
    