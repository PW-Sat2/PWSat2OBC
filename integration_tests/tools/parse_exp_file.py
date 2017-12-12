import argparse
import os
import sys
from binascii import hexlify
import pprint

try:
    from experiment_file import ExperimentFileParser
except ImportError:
    sys.path.append(os.path.join(os.path.dirname(__file__), '..'))
    from experiment_file import ExperimentFileParser


def read_all(p):
    with open(p, 'rb') as f:
        return f.read()


argparser = argparse.ArgumentParser()
argparser.add_argument('file', help="File with experiment data")
argparser.add_argument('outfile', help="Outfile")

args = argparser.parse_args()
data = read_all(args.file)

cut = 0
result = [[]]
while len(result[0]) == 0:
    result = ExperimentFileParser.parse_partial(data)
    data = data[1:]
    cut+=1
print(cut)
print(result[0])

with open(args.outfile, 'a') as f:
    print 'Parsed data:'
    for p in result[0]:
        pprint.pprint(p, f)

if len(result[1]) > 0:
    print 'Remaining data (first 10 bytes, total {} not parsed)'.format(len(result[1]))
    part = result[1][0:min(10, len(result[1]))]
    print hexlify(part)
else:
    print ''
