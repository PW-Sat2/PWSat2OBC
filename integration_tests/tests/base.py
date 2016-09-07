import unittest

from system import System

try:
    from config import config
except ImportError as e:
    raise ImportError(
        "Error loading config: %s. Did you forget to add <build>/integration_tests to PYTHONPATH?" % e.message)

mock_com = config['MOCK_COM']
obc_com = config['OBC_COM']
payload_com = config['PAYLOAD_COM']


class BaseTest(unittest.TestCase):
    def setUp(self):
        self.system = System(mock_com, obc_com, payload_com)

    def tearDown(self):
        self.system.close()
