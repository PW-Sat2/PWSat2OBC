from datetime import timedelta, datetime

from base import RestartPerTest
from system import runlevel
from utils import TestEvent
from telecommand.sads import DeploySolarArrayTelecommand


class Test_SADSTest(RestartPerTest):
    @runlevel(2)
    def test_happy_path(self):
        minutes_2 = timedelta(minutes=2)

        now = datetime.now()
        self.system.rtc.set_response_time(now)

        tk_main_lcl_enabled = TestEvent()
        tk_main_lcl_disabled = TestEvent()

        tk_red_lcl_enabled = TestEvent()
        tk_red_lcl_disabled = TestEvent()

        main_burn_switch = TestEvent()
        red_burn_switch = TestEvent()

        self.system.comm.put_frame(DeploySolarArrayTelecommand(0x22))

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
