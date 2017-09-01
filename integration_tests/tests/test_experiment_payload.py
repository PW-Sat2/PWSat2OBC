from unittest import skip

from obc.experiments import ExperimentType
from response_frames.common import ExperimentSuccessFrame
from system import clear_state
from telecommand import PerformPayloadCommissioningExperiment
from tests.base import RestartPerTest


class PayloadExperimentTest(RestartPerTest):
    @clear_state()
    @skip('Manual test')
    def test_perform_payload_experiment(self):
        self.system.obc.abort_experiment()
        self.system.obc.wait_for_experiment(None, timeout=5)

        self.system.comm.put_frame(PerformPayloadCommissioningExperiment(
            correlation_id=38,
            file_name='/payload_test'
        ))

        response = self.system.comm.get_frame(5)

        self.assertIsInstance(response, ExperimentSuccessFrame)
        self.assertEqual(response.correlation_id, 38)

        self.system.obc.wait_for_experiment(ExperimentType.Payload, timeout=20)

        self.system.obc.wait_for_experiment(None, timeout=3600)