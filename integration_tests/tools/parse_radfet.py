import argparse
import os
import sys
from binascii import hexlify
import pprint
import datetime

try:
    from experiment_file import ExperimentFileParser
except ImportError:
    sys.path.append(os.path.join(os.path.dirname(__file__), '..'))
    from experiment_file import ExperimentFileParser


def read_all(p):
    with open(p, 'rb') as f:
        return f.read()


def lsb_to_voltage(lsb):
    return lsb*2500.0/2**24

def lmt87(lsb):
    voltage = lsb*4100.0/2**12
    temperature = (voltage - 2637)/(-13.6)
    return temperature

def write_measurement_line(path, time, status, temperature, voltages, other_temp):
    with open(path + "_parsed.csv", 'a') as f:
        f.writelines("{};{};{};{};{};{};{};{};\n".format(time, status, temperature, voltages[0], voltages[1], voltages[2], other_temp['SADS'], other_temp['Supply']))

    with open(path + "_parsed_converted.csv", 'a') as f:
        f.writelines("{};{};{};{};{};{};{};{};\n".format(time, status, lsb_to_voltage(temperature/2.0), lsb_to_voltage(voltages[0]), lsb_to_voltage(voltages[1]), lsb_to_voltage(voltages[2]), other_temp['SADS'], lmt87(other_temp['Supply'])))


argparser = argparse.ArgumentParser()
argparser.add_argument('file', help="File with input experiment data (bin)")

args = argparser.parse_args()


result = ExperimentFileParser.parse_partial(read_all(args.file))

for p in result[0]:
    if p == 'Synchronization':
        continue

    try:
        time = p['time']
    except:
        pass

    try:
        status = p['RadFET']['Status']
        temperature = p['RadFET']['Temperature']
        voltages = p['RadFET']['Voltages']
    except:
        pass

    try:
        other_temo = p['Temperatures Supply & SADS']
        write_measurement_line(args.file, time, status, temperature, voltages, other_temo)
    except:
        pass



if len(result[1]) > 0:
    print 'Remaining data (first 10 bytes, total {} not parsed)'.format(len(result[1]))
    part = result[1][0:min(10, len(result[1]))]
    print hexlify(part)
else:
    print ''
