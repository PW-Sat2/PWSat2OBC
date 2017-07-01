from parser import Parser


class ExperimentTelemetryParser(Parser):
    def __init__(self, tree_control):
        Parser.__init__(self, tree_control, 'Experiments')

    def get_bit_count(self):
        return 8+8+8

    def parse(self, address, bits):
        self.append_byte(address, bits, 'LEOP Experiment State', 1)
        self.append_byte(address, bits, 'Detumbling Experiment State', 1)
        self.append_byte(address, bits, 'Sun Pointing Experiment State', 1)
        self.append_byte(address, bits, 'Sun Sensor Experiment State', 1)
        self.append_byte(address, bits, 'Camera Experiment State', 1)
        self.append_byte(address, bits, 'RAD FET Experiment State', 1)
        self.append_byte(address, bits, 'SADS Experiment State', 1)
        self.append_byte(address, bits, 'Sail Experiment State', 1)
        self.append_byte(address, bits, 'Experiment Startup Result', 8)
        self.append_byte(address, bits, 'Last Experiment Iteration Status', 8)
