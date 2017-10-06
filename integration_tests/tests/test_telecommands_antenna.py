from datetime import timedelta

from obc.boot import SelectRunlevel
from response_frames.operation import OperationSuccessFrame
from system import runlevel, clear_state
from telecommand.antenna import StopAntennaDeployment
from tests.base import BaseTest, RestartPerTest
from utils import TestEvent


class TestTelecommandsAntenna(RestartPerTest):
    @runlevel(1)
    @clear_state()
    def test_disable_antenna_deployment(self):
        self.system.obc.runlevel_start_comm()
        being_deployed = TestEvent()

        self.system.primary_antenna.on_begin_deployment = being_deployed.set

        t = timedelta(minutes=41)

        for i in xrange(0, 110):
            t += timedelta(seconds=60)

            self.system.obc.jump_to_time(t)
            self.system.obc.run_mission()

        self.assertTrue(being_deployed.wait_for_change(1), "Antenna deployment should be performed")

        self.system.obc.run_mission()

        self.system.obc._command('PUTTING FRAME')

        self.system.comm.put_frame(StopAntennaDeployment(0x22))

        ack = self.system.comm.get_frame(10, filter_type=OperationSuccessFrame)

        self.assertIsNotNone(ack, "Should receive confirmation")

        self.system.obc.run_mission()

        self.system.obc.run_mission()

        self.system.obc.sync_fs()
        self.system.restart(boot_chain=[SelectRunlevel(1)])

        since_start = self.system.obc.current_time()
        self.assertGreaterEqual(since_start, timedelta(minutes=40), "Should retain mission time during restart")

        deploy_started_again = TestEvent()

        self.system.primary_antenna.on_begin_deployment = deploy_started_again.set

        t = since_start

        for i in xrange(0, 20):
            t += timedelta(seconds=60)

            self.system.obc.jump_to_time(t)
            self.system.obc.run_mission()

        self.assertFalse(deploy_started_again.wait_for_change(1), "Antenna deployment should not be performed")
