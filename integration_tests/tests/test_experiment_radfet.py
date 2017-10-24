from obc.experiments import ExperimentType
from response_frames.common import ExperimentSuccessFrame
from system import clear_state
from telecommand import PerformRadFETExperiment
from tests.base import RestartPerTest


class RadFETExperimentTest(RestartPerTest):
    @clear_state()
    def test_perform_radfet_experiment(self):
        self.system.obc.abort_experiment()
        self.system.obc.wait_for_experiment(None, timeout=5)

        self.system.comm.put_frame(PerformRadFETExperiment(
            correlation_id=12,
            delay=1,
            samples_count=4,
            output_file_name='/exp'
        ))

        response = self.system.comm.get_frame(5, filter_type=ExperimentSuccessFrame)

        self.assertIsInstance(response, ExperimentSuccessFrame)
        self.assertEqual(response.correlation_id, 12)

        self.system.obc.wait_for_experiment(ExperimentType.RadFET, timeout=20)

        self.system.obc.wait_for_experiment(None, timeout=300)