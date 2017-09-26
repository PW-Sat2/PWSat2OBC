import logging
import struct

from nose.tools import nottest

from obc.experiments import ExperimentType
from system import auto_power_on, runlevel
from tests.base import BaseTest, RestartPerTest


class ExperimentsTest(RestartPerTest):
    @auto_power_on(False)
    def __init__(self, methodName='runTest'):
        super(ExperimentsTest, self).__init__(methodName)

    @runlevel(1)
    def test_should_conduct_experiment(self):
        iterations_count = 7

        self.power_on_obc()

        self.system.obc.set_fibo_iterations(iterations_count)
        self.system.obc.request_experiment(ExperimentType.Fibo)

        state = self.system.obc.experiment_info()
        self.assertEqual(state.Requested, ExperimentType.Fibo)
        self.assertIsNone(state.Current)

        self.system.obc.run_mission()

        self.system.obc.wait_for_experiment_iteration(1, 15)

        state = self.system.obc.experiment_info()
        self.assertIsNone(state.Requested)
        self.assertEqual(state.Current, ExperimentType.Fibo)

        for i in xrange(iterations_count - 1):
            self.system.obc.wait_for_experiment_iteration(i + 1, 5)
            self.system.obc.run_mission()

        self.system.obc.wait_for_experiment(None, timeout=15)

        state = self.system.obc.experiment_info()
        self.assertIsNone(state.Requested)
        self.assertIsNone(state.Current)

        files = self.system.obc.list_files('/')
        self.assertIn('fibo.dat', files, 'Experiment result file is not present')

        result = self.system.obc.read_file('/fibo.dat')

        self.assertEqual(len(result), 7 * 4)

        unpacked = struct.unpack('<' + 'L' * iterations_count, result)
        self.assertEqual(unpacked, (1, 1, 2, 3, 5, 8, 13))

    @runlevel(1)
    def test_should_abort_experiment(self):
        self.power_on_obc()
        log = logging.getLogger("TEST")
        self.system.obc.set_fibo_iterations(10)
        self.system.obc.request_experiment(ExperimentType.Fibo)

        log.info("1")

        for i in xrange(2):
            log.info("1.%d (run_mission)", i)
            self.system.obc.run_mission()
            log.info("1.%d (wait)", i)
            self.system.obc.wait_for_experiment_iteration(i + 1, 10)

        log.info("2")

        self.system.obc.abort_experiment()

        log.info("3")

        for i in xrange(2):
            self.system.obc.run_mission()

        log.info("4")

        self.system.obc.wait_for_experiment(None, timeout=10)

        log.info("5")

        files = self.system.obc.list_files('/')
        self.assertIn('fibo.dat', files, 'Experiment result file is not present')

        log.info("6")

        result = self.system.obc.read_file('/fibo.dat')
        self.assertEqual(len(result), 2 * 4)

        unpacked = struct.unpack('<' + 'L' * 2, result)
        self.assertEqual(unpacked, (1, 1))

    @runlevel(1)
    def test_aborting_when_no_experiment_is_running_has_no_effect(self):
        self.power_on_obc()

        self.system.obc.abort_experiment()

        self.system.obc.run_mission()

        self.system.obc.set_fibo_iterations(2)
        self.system.obc.request_experiment(ExperimentType.Fibo)
        self.system.obc.run_mission()
        self.system.obc.wait_for_experiment(ExperimentType.Fibo, 15)
        self.system.obc.wait_for_experiment_iteration(1, 15)

        self.system.obc.run_mission()
        self.system.obc.wait_for_experiment_iteration(2, 15)
        self.system.obc.wait_for_experiment(None, 15)
