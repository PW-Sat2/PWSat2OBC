import os
import logging

from system import System

logging.basicConfig(level=logging.DEBUG)

mock_com = os.environ.get('MOCK_COM')
obc_com = os.environ.get('OBC_COM')

print "Using Mock Serial port: %s" % mock_com
print "Using OBC Serial port: %s" % obc_com

system = System(mock_com, obc_com)