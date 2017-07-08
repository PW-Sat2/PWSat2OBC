from devices import *
from system import runlevel
from tests.base import RestartPerSuite


@runlevel(1)
class Test_Imtq(RestartPerSuite):
    def test_bdot(self):
        self.assertEqual(self.system.imtq.mode.mode, Imtq.Mode.Type.idle)
        self.system.obc.bdot(2)
        self.assertEqual(self.system.imtq.mode.mode, Imtq.Mode.Type.detumble)

    def test_read_magnetometer(self):
        self.system.imtq.mtm_measurement = [100, 200, -300]
        res = self.system.obc.read_magnetometer()
        self.assertEqual(res, [100, 200, -300])
