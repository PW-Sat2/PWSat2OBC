import argparse
import imp

parser = argparse.ArgumentParser()

parser.add_argument("config", help="Config file")
args = parser.parse_args()

config = imp.load_source('config', args.config)

print config.config