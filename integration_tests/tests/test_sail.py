from datetime import timedelta, datetime

from base import RestartPerTest
from system import runlevel, clear_state
from utils import TestEvent


class Test_SailTest(RestartPerTest):
    @runlevel(1)
    def test_pingpong(self):
        l = self.system.obc.ping()

        self.assertEqual("pong", l)

        boot_params = self.system.obc._command("boot_params")

        p = boot_params.find("runlevel=1")

        self.assertGreater(p, 0)

    @runlevel(2)
    @clear_state()
    def test_happy_path(self):
        days_40 = timedelta(days=40)
        minutes_2 = timedelta(minutes=2)

        now = datetime.now()
        self.system.rtc.set_response_time(now)
        self.system.obc.jump_to_time(days_40)

        tk_main_lcl_enabled = TestEvent()
        tk_main_lcl_disabled = TestEvent()

        tk_red_lcl_enabled = TestEvent()
        tk_red_lcl_disabled = TestEvent()

        main_burn_switch = TestEvent()
        red_burn_switch = TestEvent()

        self.system.eps.TKmain.on_enable = tk_main_lcl_enabled.set
        self.system.eps.TKmain.on_disable = tk_main_lcl_disabled.set

        self.system.eps.TKred.on_enable = tk_red_lcl_enabled.set
        self.system.eps.TKred.on_disable = tk_red_lcl_disabled.set

        self.system.eps.SAILmain.on_enable = main_burn_switch.set
        self.system.eps.SAILred.on_enable = red_burn_switch.set

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

        tk_main_lcl_enabled.reset()

        self.system.obc.sync_fs()
        self.system.restart(boot_chain=[])

        self.assertTrue(tk_main_lcl_enabled.wait_for_change(120))
