import logging
import os
import unittest

from devices import *
from i2cMock import I2CMock
from obc import OBC, SerialPortTerminal

mock_com = os.environ.get('MOCK_COM')
obc_com = os.environ.get('OBC_COM')


class BaseTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        logging.basicConfig(level=logging.DEBUG)

    def setUp(self):
        self.obc = OBC(SerialPortTerminal(obc_com))
        self.eps = EPSDevice()
        self.transmitter = TransmitterDevice()
        self.receiver = ReceiverDevice()

        self.i2c = I2CMock(mock_com)

        self.i2c.add_device(self.eps)
        self.i2c.add_device(self.transmitter)
        self.i2c.add_device(self.receiver)

        self.i2c.start()

        self.obc.reset()

    def tearDown(self):
        self.i2c.close()

        self.obc.close()

