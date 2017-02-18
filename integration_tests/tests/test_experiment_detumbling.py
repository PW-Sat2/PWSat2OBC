from datetime import timedelta

import telecommand
from obc.experiments import ExperimentType
from system import auto_power_on
from tests.base import BaseTest
from utils import TestEvent


class TestExperimentDetumbling(BaseTest):
    @auto_power_on(auto_power_on=False)
    def __init__(self, *args, **kwargs):
        super(TestExperimentDetumbling, self).__init__(*args, **kwargs)

    def _start(self):
        e = TestEvent()

        def on_reset(_):
            e.set()
        self.system.comm.on_hardware_reset = on_reset

        self.system.obc.power_on(clean_state=True)
        self.system.obc.wait_to_start()

        e.wait_for_change(1)

    def test_should_perform_experiment(self):
        self._start()

        self.system.comm.put_frame(telecommand.PerformDetumblingExperiment(duration=timedelta(hours=4)))

        self.system.obc.wait_for_experiment(ExperimentType.Detumbling, 40)

        self.system.obc.advance_time(timedelta(hours=4).total_seconds() * 1000)

        self.system.obc.wait_for_experiment(None, 20)
