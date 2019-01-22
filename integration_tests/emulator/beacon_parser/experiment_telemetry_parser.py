from emulator.beacon_parser.units import enum
from experiment_type import ExperimentType, StartResult, IterationResult
from parser import CategoryParser


class ExperimentTelemetryParser(CategoryParser):
    def __init__(self, reader, store):
        CategoryParser.__init__(self, '09: Experiments', reader, store)

    def get_bit_count(self):
        return 4+8+8

    def parse(self):
        self.append('Current experiment code', 4, value_type=enum(ExperimentType))
        self.append('Experiment Startup Result', 8, value_type=enum(StartResult))
        self.append('Last Experiment Iteration Status', 8, value_type=enum(IterationResult))
