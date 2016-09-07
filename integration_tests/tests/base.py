import logging
import os
import unittest
import colorlog

from system import System

mock_com = os.environ.get('MOCK_COM')
obc_com = os.environ.get('OBC_COM')
payload_com = os.environ.get('PAYLOAD_COM')


class BaseTest(unittest.TestCase):
    def setUp(self):
        self.system = System(mock_com, obc_com, payload_com)

    def tearDown(self):
        self.system.close()
