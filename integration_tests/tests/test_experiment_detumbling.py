import logging
from datetime import timedelta, datetime

import telecommand
from obc.experiments import ExperimentType
from response_frames.common import ExperimentSuccessFrame
from system import auto_power_on, runlevel
from tests.base import RestartPerTest
from utils import TestEvent


class TestExperimentDetumbling(RestartPerTest):
    @auto_power_on(auto_power_on=False)
    def __init__(self, *args, **kwargs):
        super(TestExperimentDetumbling, self).__init__(*args, **kwargs)

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
    # @skip('Mock is unable to pass self-test')
    def test_should_perform_experiment(self):
        self._start()

        power_on = TestEvent()
        power_off = TestEvent()

        self.system.eps.IMTQ.on_enable = power_on.set
        self.system.eps.IMTQ.on_disable = power_off.set

        log = logging.getLogger("TEST")

        start_time = datetime.now()
        self.system.rtc.set_response_time(start_time)

        log.info('Sending telecommand')
        self.system.comm.put_frame(telecommand.PerformDetumblingExperiment(correlation_id=5, duration=timedelta(hours=4), sampling_interval=timedelta(seconds=2)))

        response = self.system.comm.get_frame(5, filter_type=ExperimentSuccessFrame)
        self.assertIsInstance(response, ExperimentSuccessFrame)

        log.info('Waiting for experiment')
        self.system.obc.wait_for_experiment(ExperimentType.Detumbling, 40)
        self.system.obc.wait_for_experiment_iteration(1, 30)
        self.assertTrue(power_on.wait_for_change(5), "IMTQ should be powered on")

        log.info('Advancing time')
        self.system.obc.advance_time(timedelta(hours=4, minutes=1))
        self.system.rtc.set_response_time(start_time + timedelta(hours=4, minutes=1))

        log.info('Waiting for experiment finish')
        self.system.obc.wait_for_experiment(None, 25)
        self.assertTrue(power_off.wait_for_change(5), "IMTQ should be powered off")
