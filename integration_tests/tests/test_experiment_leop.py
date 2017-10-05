import logging
from datetime import timedelta

from obc.experiments import ExperimentType, StartResult
from system import auto_power_on, runlevel, clear_state
from tests.base import RestartPerTest
from utils import TestEvent


class TestExperimentLEOP(RestartPerTest):
    @auto_power_on(auto_power_on=False)
    def __init__(self, *args, **kwargs):
        super(TestExperimentLEOP, self).__init__(*args, **kwargs)

    def _start(self):
        e = TestEvent()

        def on_reset(_):
            e.set()
            return False

        self.system.comm.on_hardware_reset = on_reset

        self.power_on_obc()
        self.system.obc.wait_to_start()

        e.wait_for_change(1)

    @runlevel(2)
    @clear_state()
    def test_should_perform_experiment(self):
        self._start()

        self.system.obc.write_file('/telemetry.current', 'ABC')
        files = self.system.obc.list_files('/')
        self.assertIn('telemetry.current', files, 'Telemetry file not created')

        log = logging.getLogger("TEST")
        files = self.system.obc.remove_file('/leop')
        self.assertNotIn('leop', files, 'Experiment file not deleted')
        files = self.system.obc.remove_file('/telemetry.leop')
        self.assertNotIn('telemetry.leop', files, 'Telemetry file not deleted')

        log.info('Setting time inside experiment time slot')
        self.system.obc.jump_to_time(timedelta(minutes=5))

        self.system.obc.request_experiment(ExperimentType.LEOP)
        self.system.obc.wait_for_experiment(ExperimentType.LEOP, 40)

        log.info('Advancing time')
        self.system.obc.jump_to_time(timedelta(hours=4, minutes=5))

        log.info('Waiting for experiment finish')
        self.system.obc.wait_for_experiment(None, 40)

        files = self.system.obc.list_files('/')
        self.assertIn('leop', files, 'Experiment file not created')
        self.assertIn('telemetry.leop', files, 'Telemetry file not copied')

    @runlevel(2)
    @clear_state()
    def test_should_not_perform_experiment_after_time(self):
        self._start()

        log = logging.getLogger("TEST")

        log.info('Setting time after experiment time slot')
        self.system.obc.jump_to_time(timedelta(hours=4, minutes=5))

        self.system.obc.request_experiment(ExperimentType.LEOP)
        self.system.obc.run_mission()
        result = self.system.obc.experiment_info()

        self.assertEqual(result.LastStartResult, StartResult.Failure)

    @runlevel(3)
    @clear_state()
    def test_should_start_automatically(self):
        self._start()

        log = logging.getLogger("TEST")

        # skip initial 60s idle period
        self.system.obc.jump_to_time(timedelta(minutes=2))

        self.system.obc.wait_for_experiment_started(ExperimentType.LEOP, 60)
        result = self.system.obc.experiment_info()

        self.assertEqual(result.LastStartResult, StartResult.Success)
