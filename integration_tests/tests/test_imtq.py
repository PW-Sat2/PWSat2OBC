from devices import *
from tests.base import BaseTest
import unittest


class Test_Imtq(BaseTest):
    @unittest.skip("not working on current mock")
    def test_bdot(self):
        self.assertEqual(self.system.imtq.mode.mode, Imtq.Mode.Type.idle)
        self.system.obc.bdot(2)
        self.assertEqual(self.system.imtq.mode.mode, Imtq.Mode.Type.detumble)

    @unittest.skip("not working on current mock")
    def test_read_magnetometer(self):
        self.system.imtq.mtm_measurement = [100, 200, -300]
        res = self.system.obc.read_magnetometer()
        self.assertEqual(res, [100, 200, -300])
