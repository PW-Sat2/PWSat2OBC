from tests.base import BaseTest
import random


class Test_Gyro(BaseTest):
    def test_init(self):
        self.system.gyro.reset()
        self.assertFalse(self.system.gyro.initialised)
        self.system.obc.gyro_init()
        self.assertTrue(self.system.gyro.initialised)

    def test_read_fail_before_init(self):
        self.system.gyro.reset()

        try:
            self.system.obc.gyro_read()
            self.fail("Gyro read didn't fail despite not initialised device!")
        except ValueError:
            pass

    def test_init_and_read(self):
        self.system.gyro.reset()
        self.system.obc.gyro_init()
        self.system.obc.gyro_read()

    def test_init_and_read_correct_values(self):
        self.system.gyro.reset()
        self.system.obc.gyro_init()
        for i in xrange(10):
            rand = lambda: random.randint(-2**15, 2**15)
            x = rand()
            y = rand()
            z = rand()
            temp = rand()
            self.system.gyro.set(x, y, z, temp)

            self.assertEqual(self.system.obc.gyro_read(), [x, y, z, temp])

    def test_init_and_read_multiple_times(self):
        for i in xrange(10):
            self.system.gyro.reset()
            self.system.obc.gyro_init()

            rand = lambda: random.randint(-2**15, 2**15)
            x = rand()
            y = rand()
            z = rand()
            temp = rand()
            self.system.gyro.set(x, y, z, temp)

            self.assertEqual(self.system.obc.gyro_read(), [x, y, z, temp])
