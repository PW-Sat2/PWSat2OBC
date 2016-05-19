import logging
import os
import unittest

from system import System

mock_com = os.environ.get('MOCK_COM')
obc_com = os.environ.get('OBC_COM')


class BaseTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        logging.basicConfig(level=logging.DEBUG)

    def setUp(self):
        self.system = System(mock_com, obc_com)

    def tearDown(self):
        self.system.close()
