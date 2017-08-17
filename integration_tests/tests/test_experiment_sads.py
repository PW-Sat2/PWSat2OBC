from datetime import timedelta, datetime

from obc.experiments import ExperimentType
from system import runlevel, clear_state
from response_frames.operation import OperationSuccessFrame
from telecommand.experiments import PerformSADSExperiment
from tests.base import RestartPerTest
from utils import TestEvent


@runlevel(1)
class TestExperimentSADS(RestartPerTest):

    def startup(self):
        self.system.obc.runlevel_start_comm()
        self.system.obc.jump_to_time(timedelta(hours=41))

    def test_experiment_startup(self):
        self.startup()
        self.system.obc.jump_to_time(timedelta(hours=41))

        self.system.comm.put_frame(PerformSADSExperiment(10))

        frame = self.system.comm.get_frame(20)

        self.assertIsInstance(frame, OperationSuccessFrame)
        self.assertEqual(frame.correlation_id, 10);

        self.system.obc.wait_for_experiment_started(ExperimentType.SADS, 60)
        self.system.obc.wait_for_experiment_iteration(1, 3)

    @clear_state()
    @runlevel(2)
    def test_experiment_execution(self):
        minutes_2 = timedelta(minutes=2)

        self.startup()
        self.system.obc.jump_to_time(timedelta(hours=41))

        self.system.comm.put_frame(PerformSADSExperiment(10))

        self.system.obc.wait_for_experiment_started(ExperimentType.SADS, 60)
        self.system.obc.wait_for_experiment_iteration(1, 20)

        now = datetime.now()
        self.system.rtc.set_response_time(now)

        tk_main_lcl_enabled = TestEvent()
        tk_main_lcl_disabled = TestEvent()

        tk_red_lcl_enabled = TestEvent()
        tk_red_lcl_disabled = TestEvent()

        main_burn_switch = TestEvent()
        red_burn_switch = TestEvent()

        self.system.comm.put_frame(PerformSADSExperiment(0x22))

        self.system.eps.TKmain.on_enable = tk_main_lcl_enabled.set
        self.system.eps.TKmain.on_disable = tk_main_lcl_disabled.set

        self.system.eps.TKred.on_enable = tk_red_lcl_enabled.set
        self.system.eps.TKred.on_disable = tk_red_lcl_disabled.set

        self.system.eps.SADSmain.on_enable = main_burn_switch.set
        self.system.eps.SADSred.on_enable = red_burn_switch.set

        self.assertTrue(tk_main_lcl_enabled.wait_for_change(16))
        self.assertTrue(main_burn_switch.wait_for_change(10))

        self.system.rtc.set_response_time(now + minutes_2)
        self.system.obc.advance_time(timedelta(minutes=2))

        self.assertTrue(tk_main_lcl_disabled.wait_for_change(16))
        self.assertTrue(tk_red_lcl_enabled.wait_for_change(16))
        self.assertTrue(red_burn_switch.wait_for_change(10))

        self.system.rtc.set_response_time(now + 2 * minutes_2)
        self.system.obc.advance_time(timedelta(minutes=2))

        self.assertTrue(tk_main_lcl_disabled.wait_for_change(16))
        self.assertTrue(tk_red_lcl_disabled.wait_for_change(16))

        files = self.system.obc.list_files('/')
        self.assertIn('sads.exp', files, 'Experiment file not created')

