from system import auto_power_on, runlevel
from tests.base import RestartPerSuite
from utils import TestEvent

@runlevel(1)
class StartupTest(RestartPerSuite):

    def wait_for_lcl(self, lcl, message):
        ev = TestEvent()
        lcl.on_disable = ev.set
        self.power_on_obc()
        self.assertTrue(ev.wait_for_change(1), message)

    @auto_power_on(auto_power_on=False)
    def test_reset_sens_lcl(self):
        self.wait_for_lcl(self.system.eps.SENS, "Sens LCL Should be reset on startup")

    @auto_power_on(auto_power_on=False)
    def test_reset_cam_nadir_lcl(self):
        self.wait_for_lcl(self.system.eps.CamNadir, "Cam Nadir LCL Should be reset on startup")

    @auto_power_on(auto_power_on=False)
    def test_reset_cam_wing_lcl(self):
        self.wait_for_lcl(self.system.eps.CamWing, "Cam Wing Should be reset on startup")

    @auto_power_on(auto_power_on=False)
    def test_reset_ant_lcl(self):
        self.wait_for_lcl(self.system.eps.ANTenna, "ANT Should be reset on startup")

    @auto_power_on(auto_power_on=False)
    def test_reset_ant_red_lcl(self):
        self.wait_for_lcl(self.system.eps.ANTennaRed, "ANT Red LCL Should be reset on startup")


