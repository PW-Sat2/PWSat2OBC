import os
import threading
import unittest

from devices import EPSDevice
from i2cMock import I2CMock
from obc import OBC, SerialPortTerminal

mock_com = os.environ.get('MOCK_COM')
obc_com = os.environ.get('OBC_COM')


class BaseTest(unittest.TestCase):
    @classmethod
    def setUpClass(self):
        self.obc = OBC(SerialPortTerminal(obc_com))
        self.eps = EPSDevice()
        self.i2c = I2CMock(mock_com)

        self.i2c.add_device(self.eps)

        self.thread = threading.Thread(target=I2CMock.run, args=(self.i2c,))

        self.thread.start()

    def setUp(self):
        self.obc.reset()

    @classmethod
    def tearDownClass(self):
        self.i2c.close()

        self.thread.join()

        del self.i2c