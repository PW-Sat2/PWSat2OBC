import struct

from obc.experiments import ExperimentType
from system import auto_power_on
from tests.base import BaseTest


class ExperimentsTest(BaseTest):
    @auto_power_on(False)
    def __init__(self, methodName='runTest'):
        super(ExperimentsTest, self).__init__(methodName)

    def begin(self):
        self.power_on_and_wait()
        self.system.obc.suspend_mission()

    def test_should_conduct_experiment(self):
        iterations_count = 7

        self.begin()

        self.system.obc.set_fibo_iterations(iterations_count)
        self.system.obc.request_experiment(ExperimentType.Fibo)

        state = self.system.obc.experiment_info()
        self.assertEqual(state.Requested, ExperimentType.Fibo)
        self.assertIsNone(state.Current)

        self.system.obc.run_mission()

        self.system.obc.wait_for_experiment_iteration(1, 3)

        state = self.system.obc.experiment_info()
        self.assertIsNone(state.Requested)
        self.assertEqual(state.Current, ExperimentType.Fibo)

        for i in xrange(iterations_count - 1):
            self.system.obc.wait_for_experiment_iteration(i + 1, 3)
            self.system.obc.run_mission()

        self.system.obc.wait_for_experiment(None, timeout=5)

        state = self.system.obc.experiment_info()
        self.assertIsNone(state.Requested)
        self.assertIsNone(state.Current)

        files = self.system.obc.list_files('/a')
        self.assertIn('fibo.dat', files, 'Experiment result file is not present')

        result = self.system.obc.read_file('/a/fibo.dat')

        self.assertEqual(len(result), 7 * 4)

        unpacked = struct.unpack('<' + 'L' * iterations_count, result)
        self.assertEqual(unpacked, (1, 1, 2, 3, 5, 8, 13))

    def test_should_abort_experiment(self):
        self.begin()

        self.system.obc.set_fibo_iterations(10)
        self.system.obc.request_experiment(ExperimentType.Fibo)

        for i in xrange(2):
            self.system.obc.run_mission()
            self.system.obc.wait_for_experiment_iteration(i + 1, 3)

        self.system.obc.abort_experiment()

        for i in xrange(2):
            self.system.obc.run_mission()

        self.system.obc.wait_for_experiment(None, timeout=3)

        files = self.system.obc.list_files('/a')
        self.assertIn('fibo.dat', files, 'Experiment result file is not present')

        result = self.system.obc.read_file('/a/fibo.dat')
        self.assertEqual(len(result), 2 * 4)

        unpacked = struct.unpack('<' + 'L' * 2, result)
        self.assertEqual(unpacked, (1, 1))

    def test_aborting_when_no_experiment_is_running_has_no_effect(self):
        self.begin()

        self.system.obc.abort_experiment()

        self.system.obc.run_mission()

        self.system.obc.set_fibo_iterations(100)
        self.system.obc.request_experiment(ExperimentType.Fibo)
        self.system.obc.run_mission()
        self.system.obc.wait_for_experiment(ExperimentType.Fibo, 15)
