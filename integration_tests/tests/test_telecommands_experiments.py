from obc.experiments import ExperimentType
from system import auto_power_on, runlevel
from telecommand import AbortExperiment
from tests.base import BaseTest
from utils import TestEvent


class TestExperimentsTelecommands(BaseTest):
    @auto_power_on(auto_power_on=False)
    def __init__(self, *args, **kwargs):
        super(TestExperimentsTelecommands, self).__init__(*args, **kwargs)

    def _start(self):
        e = TestEvent()

        def on_reset(_):
            e.set()
            return False
        self.system.comm.on_hardware_reset = on_reset

        self.system.obc.power_on(clean_state=True)
        self.system.obc.wait_to_start()

        e.wait_for_change(1)

    @runlevel(2)
    def test_should_abort_experiment(self):
        self._start()

        self.system.obc.set_fibo_iterations(10)
        self.system.obc.request_experiment(ExperimentType.Fibo)
        self.system.obc.wait_for_experiment(ExperimentType.Fibo, 15)

        self.system.comm.put_frame(AbortExperiment())

        self.system.obc.wait_for_experiment(None, 15)

    @runlevel(2)
    def test_should_be_able_to_run_next_experiment_after_previous_aborted(self):
        self._start()

        self.system.obc.set_fibo_iterations(10)
        self.system.obc.request_experiment(ExperimentType.Fibo)
        self.system.obc.wait_for_experiment(ExperimentType.Fibo, 15)

        self.system.comm.put_frame(AbortExperiment())

        self.system.obc.wait_for_experiment(None, 15)

        self.system.obc.request_experiment(ExperimentType.Fibo)

        self.system.obc.wait_for_experiment(ExperimentType.Fibo, 15)
        self.system.obc.abort_experiment()
        self.system.obc.wait_for_experiment(None, 15)
