from devices import *
from system import runlevel
from tests.base import RestartPerSuite
from utils import TestEvent


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

    def test_get_detumble_data(self):
        self.system.imtq.bdot_data = [11, 22, 33]
        self.system.imtq.mtm_measurement = [100, 200, -300]
        self.system.imtq.coil_current = [10, -20, 30]
        self.system.imtq.commanded_dipole = [0, 1, 2]
        res = self.system.obc.get_detumble_data()
        self.assertEqual(res, [100, 200, -300, 100, 200, -300, 11, 22, 33, 0, 1, 2, 10, -20, 30, 10, -20, 30])

    def test_self_test(self):
        test_started = TestEvent()
        self.system.imtq.on_start_self_test = test_started.set

        self_test_result = self.system.obc._command('imtq PerformSelfTest true')

        self.assertMultiLineEqual(self_test_result, """0 0 0 0 0 0 0 0 0 0 0 0 0 0
0 0 0 0 0 0 0 0 0 0 0 0 0 0
0 0 0 0 0 0 0 0 0 0 0 0 0 0
0 0 0 0 0 0 0 0 0 0 0 0 0 0
0 0 0 0 0 0 0 0 0 0 0 0 0 0
0 0 0 0 0 0 0 0 0 0 0 0 0 0
0 0 0 0 0 0 0 0 0 0 0 0 0 0
0 0 0 0 0 0 0 0 0 0 0 0 0 0""")

        self.assertTrue(test_started.wait_for_change(0), "Self test should be started")
