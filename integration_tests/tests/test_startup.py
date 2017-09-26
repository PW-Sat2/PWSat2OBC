from system import auto_power_on, runlevel
from tests.base import RestartPerSuite
from utils import TestEvent

@runlevel(1)
class StartupTest(RestartPerSuite):

    def setup_lcl(self, lcl, message):
        ev = TestEvent()
        lcl.on_disable = ev.set
        return lambda : self.assertTrue(ev.wait_for_change(1), message)

    @auto_power_on(auto_power_on=False)
    def test_lcls_are_reset(self):
        sens = self.setup_lcl(self.system.eps.SENS, "Sens LCL Should be reset on startup")
        camNadir = self.setup_lcl(self.system.eps.CamNadir, "Cam Nadir LCL Should be reset on startup")
        camWing = self.setup_lcl(self.system.eps.CamWing, "Cam Wing Should be reset on startup")
        ant = self.setup_lcl(self.system.eps.ANTenna, "ANT Should be reset on startup")
        antRed = self.setup_lcl(self.system.eps.ANTennaRed, "ANT Red LCL Should be reset on startup")
        self.power_on_obc()
        sens()
        camNadir()
        camWing()
        ant()
        antRed()
