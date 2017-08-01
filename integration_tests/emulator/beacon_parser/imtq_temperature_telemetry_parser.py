from parser import Parser


class ImtqTemperatureTelemetryParser(Parser):
    def __init__(self, tree_control):
        Parser.__init__(self, tree_control, 'Imtq Temperature')

    def get_bit_count(self):
        return 3 * 16

    def parse(self, address, bits):
        self.append_word(address, bits, "Coil Temperature 1")
        self.append_word(address, bits, "Coil Temperature 2")
        self.append_word(address, bits, "Coil Temperature 3")
