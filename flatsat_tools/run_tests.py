import argparse
import imp
import os
import nose
import sys

base = os.path.dirname(__file__)

parser = argparse.ArgumentParser()

parser.add_argument("config", help="Config file")
parser.add_argument("tests", help="Path to tests directory")
parser.add_argument('test_args', nargs=argparse.REMAINDER)
args = parser.parse_args()

config = imp.load_source('config', args.config)

if not os.path.exists(args.tests):
    print "Unable to find tests directory '{}'".format(args.tests)
    sys.exit(1)

nose_args = ['%s -m nose' % sys.executable, '-w', args.tests] + args.test_args
print nose_args
nose.run(argv=nose_args)
