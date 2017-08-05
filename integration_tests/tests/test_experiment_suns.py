from threading import Timer
from unittest import skip

from obc.experiments import ExperimentType
from system import clear_state
from tests.base import RestartPerTest


class SunSExperimentTest(RestartPerTest):
    @clear_state()
    @skip('Manual test')
    def test_perform_suns_experiment(self):
        def on_suns_measure():
            self.system.suns.gpio_interrupt_high()

            def finish():
                self.system.suns.gpio_interrupt_low()

            t = Timer(0.1, finish)
            t.start()

        self.system.suns.on_measure = on_suns_measure

        self.system.obc.abort_experiment()
        self.system.obc.wait_for_experiment(None, timeout=5)

        self.system.obc._command("exp_suns params 1 2 4 2 5 1")
        self.system.obc._command("exp_suns file /exp")
        self.system.obc._command("exp_suns start")

        self.system.obc.wait_for_experiment(ExperimentType.SunS, timeout=20)

        self.system.obc.wait_for_experiment(None, timeout=3600)