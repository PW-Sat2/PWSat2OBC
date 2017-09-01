from unittest import skip

from obc.experiments import ExperimentType
from response_frames.common import ExperimentSuccessFrame
from system import clear_state
from telecommand import PerformCameraCommissioningExperiment
from tests.base import RestartPerTest


class CameraExperimentTest(RestartPerTest):
    @clear_state()
    @skip('Manual test')
    def test_perform_camera_experiment(self):

        self.system.obc.abort_experiment()
        self.system.obc.wait_for_experiment(None, timeout=5)

        self.system.comm.put_frame(PerformCameraCommissioningExperiment(
            correlation_id=32,
            file_name='/camera_test'
        ))

        response = self.system.comm.get_frame(5)

        self.assertIsInstance(response, ExperimentSuccessFrame)
        self.assertEqual(response.correlation_id, 32)

        self.system.obc.wait_for_experiment(ExperimentType.Camera, timeout=20)

        self.system.obc.wait_for_experiment(None, timeout=3600)