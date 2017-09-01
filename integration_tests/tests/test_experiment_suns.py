from datetime import timedelta
from unittest import skip

from obc.experiments import ExperimentType
from response_frames.common import ExperimentSuccessFrame
from system import clear_state
from telecommand import PerformSunSExperiment
from tests.base import RestartPerTest


class SunSExperimentTest(RestartPerTest):
    @clear_state()
    @skip('Manual test')
    def test_perform_suns_experiment(self):
        self.system.obc.abort_experiment()
        self.system.obc.wait_for_experiment(None, timeout=5)

        self.system.comm.put_frame(PerformSunSExperiment(
            correlation_id=34,
            gain=1,
            itime=2,
            samples_count=4,
            short_delay=timedelta(seconds=2),
            sessions_count=5,
            long_delay=timedelta(minutes=1),
            file_name='/exp'
        ))

        response = self.system.comm.get_frame(5)

        self.assertIsInstance(response, ExperimentSuccessFrame)
        self.assertEqual(response.correlation_id, 34)

        self.system.obc.wait_for_experiment(ExperimentType.SunS, timeout=20)

        self.system.obc.wait_for_experiment(None, timeout=3600)