from datetime import timedelta

from obc.boot import SelectRunlevel
from obc.experiments import ExperimentType
from system import runlevel, clear_state
from response_frames.operation import OperationSuccessFrame
from response_frames.sail_experiment import SailExperimentFrame
from telecommand.experiments import PerformSailExperiment
from tests.base import RestartPerTest
from utils import TestEvent
from nose.tools import nottest

@runlevel(1)
class TestExperimentSail(RestartPerTest):

    def startup(self):
        self.system.obc.runlevel_start_comm()
        self.system.obc.jump_to_time(timedelta(hours=41))

    @clear_state()
    def test_experiment_startup(self):
        self.startup()
        self.system.obc.jump_to_time(timedelta(hours=41))

        self.system.comm.put_frame(PerformSailExperiment(10))

        frame = self.system.comm.get_frame(20)

        self.assertIsInstance(frame, OperationSuccessFrame)
        self.assertEqual(frame.correlation_id, 10);

        self.system.obc.wait_for_experiment_started(ExperimentType.Sail, 60)
        self.system.obc.wait_for_experiment_iteration(1, 3)

    @clear_state()
    @nottest
    def test_experiment_execution(self):
        self.startup()
        self.system.obc.jump_to_time(timedelta(hours=41))

        self.system.comm.put_frame(PerformSailExperiment(10))

        self.system.obc.wait_for_experiment_started(ExperimentType.Sail, 60)
        self.system.obc.wait_for_experiment_iteration(1, 20)

        self.system.obc.jump_to_time(timedelta(hours=41, minutes = 5))

        self.system.obc.wait_for_experiment(None, 180)

    @clear_state()
    def test_auto_send(self):
        self.startup()
        self.system.obc.jump_to_time(timedelta(hours=41))

        self.system.comm.put_frame(PerformSailExperiment(10))

        frame = self.system.comm.get_frame(20)

        self.assertIsInstance(frame, OperationSuccessFrame)
        self.assertEqual(frame.correlation_id, 10)

        self.system.obc.wait_for_experiment_started(ExperimentType.Sail, 60)
        self.system.obc.wait_for_experiment_iteration(23, 5 * 20)

        frame = self.system.comm.get_frame(5)
        self.assertIsInstance(frame, SailExperimentFrame)
        self.assertEqual(frame.seq(), 0)

        frame = self.system.comm.get_frame(5)
        self.assertIsInstance(frame, SailExperimentFrame)
        self.assertEqual(frame.seq(), 1)
        
