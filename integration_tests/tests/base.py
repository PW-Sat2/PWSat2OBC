import unittest

from system import System

try:
    from config import config
except ImportError as e:
    raise ImportError(
        "Error loading config: %s. Did you forget to add <build>/integration_tests to PYTHONPATH?" % e.message)




class BaseTest(unittest.TestCase):
    def setUp(self):
        obc_com = config['OBC_COM']
        sys_bus_com = config['SYS_BUS_COM']
        payload_bus_com = config['PAYLOAD_BUS_COM']
        use_single_bus = config['SINGLE_BUS']

        self.system = System(obc_com, sys_bus_com, payload_bus_com, use_single_bus)

    def tearDown(self):
        self.system.close()
