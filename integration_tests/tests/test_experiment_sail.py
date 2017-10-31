from datetime import timedelta

from obc.experiments import ExperimentType
from system import runlevel, clear_state
from response_frames.common import ExperimentSuccessFrame
from response_frames.sail_experiment import SailExperimentFrame
from telecommand.experiments import PerformSailExperiment
from tests.base import RestartPerTest
from nose.tools import nottest


class TestExperimentSail(RestartPerTest):

    def startup(self):
        self.system.obc.runlevel_start_comm()
        self.system.obc.jump_to_time(timedelta(hours=41))

    @runlevel(2)
    @clear_state()
    def test_experiment_startup(self):
        self.startup()
        self.system.obc.jump_to_time(timedelta(hours=41))

        self.system.comm.put_frame(PerformSailExperiment(10))

        frame = self.system.comm.get_frame(20, filter_type=ExperimentSuccessFrame)

        self.assertIsInstance(frame, ExperimentSuccessFrame)
        self.assertEqual(frame.correlation_id, 10)

        self.system.obc.wait_for_experiment_started(ExperimentType.Sail, 60)
        self.system.obc.wait_for_experiment_iteration(1, 3)

    @clear_state()
    @nottest
    @runlevel(2)
    def test_experiment_execution(self):
        self.startup()
        self.system.obc.jump_to_time(timedelta(hours=41))

        self.system.comm.put_frame(PerformSailExperiment(10))

        self.system.obc.wait_for_experiment_started(ExperimentType.Sail, 60)
        self.system.obc.wait_for_experiment_iteration(1, 20)

        self.system.obc.jump_to_time(timedelta(hours=41, minutes = 5))

        self.system.obc.wait_for_experiment(None, 180)

    @clear_state()
    @runlevel(2)
    def test_auto_send(self):
        def suppress_comm_soft_reset():
            return False

        self.system.transmitter.on_reset = suppress_comm_soft_reset

        self.startup()
        self.system.obc.jump_to_time(timedelta(hours=41))

        self.system.comm.put_frame(PerformSailExperiment(10))

        frame = self.system.comm.get_frame(20, filter_type=ExperimentSuccessFrame)

        self.assertIsInstance(frame, ExperimentSuccessFrame)
        self.assertEqual(frame.correlation_id, 10)

        self.system.obc.wait_for_experiment_started(ExperimentType.Sail, 60)
        self.system.obc.wait_for_experiment_iteration(25, 5 * 20)

        frame = self.system.comm.get_frame(20, filter_type=SailExperimentFrame)
        self.assertIsInstance(frame, SailExperimentFrame)
        self.assertEqual(frame.seq(), 0)

        frame = self.system.comm.get_frame(20, filter_type=SailExperimentFrame)
        self.assertIsInstance(frame, SailExperimentFrame)
        self.assertEqual(frame.seq(), 1)
        
