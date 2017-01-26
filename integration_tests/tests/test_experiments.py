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
        self.begin()

        self.system.obc.request_experiment(ExperimentType.Fibo)

        self.system.obc.run_mission()

        for i in xrange(5):
            self.system.obc.run_mission()

        files = self.system.obc.list_files('/')
        self.assertIn('fibo.dat', files, 'Experiment result file is not present')

        result = self.system.obc.read_file('/fibo.dat')

        self.assertEqual(result, 'ABC' + 'BCD' + 'CDE' + 'DEF' + 'EFG')
