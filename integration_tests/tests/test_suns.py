from devices import *
from system import runlevel
from tests.base import RestartPerSuite


@runlevel(1)
class Test_SunS(RestartPerSuite):
    def test_bdot(self):
        self.system.suns.gpio_interrupt_high()
        res = self.system.obc.measure_suns(1, 2)
        self.assertEqual(res, [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33])
