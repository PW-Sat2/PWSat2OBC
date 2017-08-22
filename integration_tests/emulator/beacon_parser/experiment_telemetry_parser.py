from parser import CategoryParser


class ExperimentTelemetryParser(CategoryParser):
    def __init__(self, reader, store):
        CategoryParser.__init__(self, 'Experiments', reader, store)

    def get_bit_count(self):
        return 8+8+8

    def parse(self):
        self.append('Current experiment code', 4)
        self.append('Experiment Startup Result', 8)
        self.append('Last Experiment Iteration Status', 8)
