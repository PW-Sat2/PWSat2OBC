import logging
from datetime import timedelta, datetime

import telecommand
from obc.experiments import ExperimentType
from system import auto_power_on, runlevel
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
            return False
        self.system.comm.on_hardware_reset = on_reset

        self.system.obc.power_on()
        self.system.obc.wait_to_start()

        e.wait_for_change(1)

    @runlevel(2)
    def test_should_perform_experiment(self):
        self._start()

        log = logging.getLogger("TEST")

        start_time = datetime.now()
        self.system.rtc.set_response_time(start_time)

        log.info('Sending telecommand')
        self.system.comm.put_frame(telecommand.PerformDetumblingExperiment(duration=timedelta(hours=4)))

        log.info('Waiting for experiment')
        self.system.obc.wait_for_experiment(ExperimentType.Detumbling, 40)

        log.info('Advancing time')
        self.system.obc.advance_time(timedelta(hours=4, minutes=1).total_seconds() * 1000)
        self.system.rtc.set_response_time(start_time + timedelta(hours=4, minutes=1))

        log.info('Waiting for experiment finish')
        self.system.obc.wait_for_experiment(None, 20)
