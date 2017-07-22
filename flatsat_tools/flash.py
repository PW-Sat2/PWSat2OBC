import argparse
import imp
import os
import tempfile

import sys

parser = argparse.ArgumentParser()

parser.add_argument("config", help="Config file")
parser.add_argument("type", help="Which binary to flash", choices=['boot', 'pwsat'])
args = parser.parse_args()

config = imp.load_source('config', args.config)

base = os.path.dirname(__file__)

hex_file = os.path.join(base, '..', 'bin', args.type + '.hex')

if not os.path.exists(hex_file):
    print 'Hex file ({}) not found. Are you using proper scripts?'.format(hex_file)
    sys.exit(1)

f = tempfile.NamedTemporaryFile(delete=False)
f.write("""
si 1
speed auto
r
halt
loadbin "{hex_file}", 0x0
r
g
exit
""".format(hex_file=hex_file))
f.close()

os.system("\"{jlink}\" -device EFM32GG280F1024 -SelectEmuBySN {serial} -ExitOnError -CommanderScript {script}"
          .format(script=f.name, serial=config.config['JLINK_SN'], jlink=config.config['JLINK']))

os.unlink(f.name)
