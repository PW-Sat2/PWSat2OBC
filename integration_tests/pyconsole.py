import os
import logging

from system import System

try:
    from config import config
except ImportError as e:
    raise ImportError(
        "Error loading config: %s. Did you forget to add <build>/integration_tests to PYTHONPATH?" % e.message)

logging.basicConfig(level=logging.DEBUG)

mock_com = os.environ.get('MOCK_COM')
obc_com = os.environ.get('OBC_COM')

print "Using Mock Serial port: %s" % mock_com
print "Using OBC Serial port: %s" % obc_com

obc_com = config['OBC_COM']
sys_bus_com = config['SYS_BUS_COM']
payload_bus_com = config['PAYLOAD_BUS_COM']
use_single_bus = config['SINGLE_BUS']

system = System(obc_com, sys_bus_com, payload_bus_com, use_single_bus)

print "done"