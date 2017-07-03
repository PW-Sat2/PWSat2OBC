from obc.boot import SelectRunlevel
from system import runlevel, clear_state
from telecommand.antenna import StopAntennaDeployment
from tests.base import BaseTest
from utils import TestEvent


class TestTelecommandsAntenna(BaseTest):
    @runlevel(1)
    @clear_state()
    def test_disable_antenna_deployment(self):
        self.system.obc.runlevel_start_comm()
        self.system.obc.jump_to_time(41 * 60)

        being_deployed = TestEvent()

        self.system.primary_antenna.on_begin_deployment = being_deployed.set

        for _ in xrange(0, 15):
            self.system.obc.run_mission()
            self.system.primary_antenna.finish_deployment()
            self.system.backup_antenna.finish_deployment()

        self.assertTrue(being_deployed.wait_for_change(1), "Antenna deployment should be performed")

        self.system.obc.run_mission()

        self.system.comm.put_frame(StopAntennaDeployment(0x22))

        self.system.comm.transmitter.get_message_from_buffer(10)

        self.system.obc.run_mission()

        self.system.obc.run_mission()

        self.system.restart(boot_chain=[SelectRunlevel(1)])

        minutes_since_start = self.system.obc.current_time() / 60.0 / 1000.0
        self.assertGreaterEqual(minutes_since_start, 40, "Should retain mission time during restart")

        deploy_started_again = TestEvent()

        self.system.primary_antenna.on_begin_deployment = deploy_started_again.set

        self.system.obc.run_mission()
        self.system.obc.run_mission()

        self.assertFalse(deploy_started_again.wait_for_change(1), "Antenna deployment should not be performed")
