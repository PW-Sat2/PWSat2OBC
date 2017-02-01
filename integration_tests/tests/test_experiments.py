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

        self.system.obc.run_mission()

        for i in xrange(iterations_count):
            self.system.obc.run_mission()

        files = self.system.obc.list_files('/')
        self.assertIn('fibo.dat', files, 'Experiment result file is not present')

        result = self.system.obc.read_file('/fibo.dat')

        self.assertEqual(len(result), 7 * 4)

        unpacked = struct.unpack('<' + 'L' * iterations_count, result)
        self.assertEqual(unpacked, (1, 1, 2, 3, 5, 8, 13))

    def test_should_abort_experiment(self):
        self.begin()

        self.system.obc.set_fibo_iterations(10)
        self.system.obc.request_experiment(ExperimentType.Fibo)

        for i in xrange(2):
            self.system.obc.run_mission()

        self.system.obc.abort_experiment()

        for i in xrange(2):
            self.system.obc.run_mission()

        files = self.system.obc.list_files('/')
        self.assertIn('fibo.dat', files, 'Experiment result file is not present')

        result = self.system.obc.read_file('/fibo.dat')
        self.assertEqual(len(result), 2 * 4)

        unpacked = struct.unpack('<' + 'L' * 2, result)
        self.assertEqual(unpacked, (1, 1))
