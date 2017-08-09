from datetime import timedelta

from obc.boot import SelectRunlevel
from obc.experiments import ExperimentType
from system import runlevel, clear_state
from response_frames.operation import OperationSuccessFrame
from telecommand.experiments import PerformSailExperiment
from tests.base import RestartPerTest
from utils import TestEvent

@runlevel(1)
class TestExperimentSail(RestartPerTest):

    def startup(self):
        self.system.obc.runlevel_start_comm()
        self.system.obc.jump_to_time(timedelta(hours=41))

    
    def test_experiment_start(self):
        self.startup()
        self.system.comm.put_frame(PerformSailExperiment(10))

        frame = self.system.comm.get_frame(20)

        self.assertIsInstance(frame, OperationSuccessFrame)
        self.assertEqual(frame.correlation_id, 10);
        self.assertEqual(frame.response, [0]);

    def test_experiment_is_started_up(self):
        self.startup()
        self.system.obc.jump_to_time(timedelta(hours=41))

        self.system.comm.put_frame(PerformSailExperiment(10))

        self.system.obc.wait_for_experiment(ExperimentType.Sail, 20)
