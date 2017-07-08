from parser import Parser


class GyroscopeTelemetryParser(Parser):
    def __init__(self, tree_control):
        Parser.__init__(self, tree_control, 'Gyroscope')

    def get_bit_count(self):
        return 4 * 16

    def parse(self, address, bits):
        self.append_word(address, bits, "X measurement")
        self.append_word(address, bits, "Y measurement")
        self.append_word(address, bits, "Z measurement")
        self.append_word(address, bits, "Temperature")

