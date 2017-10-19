from system import runlevel
from tests.base import RestartPerTest
from utils import TestEvent


class ADCSTest(RestartPerTest):
    @runlevel(1)
    def test_should_perform_self_test_before_enabling_builtin_detumbling(self):
        bdot_started = TestEvent()
        test_started = TestEvent()
        power_on = TestEvent()

        self.system.imtq.on_start_bdot = bdot_started.set
        self.system.eps.IMTQ.on_enable = power_on.set

        def on_self_test_start(*args):
            if bdot_started.flag.is_set:
                test_started.set()

        self.system.imtq.on_start_self_test = on_self_test_start

        self.system.obc.adcs_enable_builtin()

        self.assertTrue(test_started.wait_for_change(5), "Self test should be performed before BDot")
        self.assertTrue(bdot_started.wait_for_change(5), "BDot should be enabled")
        self.assertTrue(power_on.wait_for_change(5), "LCL should be enabled")
