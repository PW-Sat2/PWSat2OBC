from devices import *
from system import runlevel
from tests.base import RestartPerSuite
from utils import RepeatedTimer


@runlevel(1)
class SunSTest(RestartPerSuite):
    def __init__(self, *args, **kwargs):
        super(SunSTest, self).__init__(*args, **kwargs)
        self._high = True
        self._timer = RepeatedTimer(1, self._tick_time)

    def test_measure(self):
        self._timer.start()
        res = self.system.obc.measure_suns(1, 2)
        self.assertEqual(res, [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33])
        self._timer.stop()

    def _tick_time(self):
        if self._high:
            self.system.suns.gpio_interrupt_low()
        else:
            self.system.suns.gpio_interrupt_high()
        self._high = not self._high
