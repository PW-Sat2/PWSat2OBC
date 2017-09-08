import unittest

import os

from os import path
from parameterized import parameterized
from build_config import config
from experiment_file import ExperimentFileParser


class TestParser(unittest.TestCase):
    EXPERIMENTS_PATH = path.join(config['REPORTS_DIR'], 'experiments')

    @parameterized.expand(map(lambda x: (x,), os.listdir(EXPERIMENTS_PATH)))
    def test_should_parse_all_files(self, experiment_file_name):
        with open(path.join(self.EXPERIMENTS_PATH, experiment_file_name), 'rb') as f:
            data = f.read()

            (_, unparsed) = ExperimentFileParser.parse_partial(data)
            self.assertEqual(unparsed, '', 'Whole file has been parsed')