import unittest
import serial
import os
import threading

from obc import OBC, SerialPortTerminal
from i2cMock import I2CMock, I2CDevice
import i2cMock


mock_com = os.environ.get('MOCK_COM') or "com45"
obc_com = os.environ.get('OBC_COM') or "com46"

INFINITY_TIME = 999999

EPS_DEVICE_ADDRESS = 12


class EPSDevice(I2CDevice):
    def __init__(self):
        self.opened = False
        return super(EPSDevice, self).__init__(EPS_DEVICE_ADDRESS)

    @i2cMock.command([0x01])
    def lcl_sail_0(self, onoff):
        self.opened = True
        print "LCL_SAIL_0: %s" % str(onoff)

    @i2cMock.command([0x02])
    def lcl_sail_1(self, onoff):
        print "LCL_SAIL_1: %s" % str(onoff)

class Test_SailTest(unittest.TestCase):
    @classmethod
    def setUpClass(self):
        self.obc = OBC(SerialPortTerminal(obc_com))
        self.eps = EPSDevice()
        self.i2c = I2CMock(mock_com)

        self.i2c.add_device(self.eps)

        self.thread = threading.Thread(target=I2CMock.run, args=(self.i2c,))

        self.thread.start()

    @classmethod
    def tearDownClass(self):
        self.i2c.close()

        self.thread.join()
        #
        # self.obc.close()

        del self.i2c

    def test_pingpong(self):
        l = self.obc.ping()

        self.assertEqual("pong", l)

    def test_jump_to_time(self):
        self.obc.jump_to_time(INFINITY_TIME)

        current_time = self.obc.current_time()

        self.assertGreaterEqual(current_time, INFINITY_TIME)

    def test_happy_path(self):
        #todo: start from T=0
        self.obc.jump_to_time(INFINITY_TIME)
        threading._sleep(1)

        self.assertTrue(self.eps.opened)
