from parser import Parser


class ImtqDipoleTelemetryParser(Parser):
    def __init__(self, tree_control):
        Parser.__init__(self, tree_control, 'Imtq Dipole')

    def get_bit_count(self):
        return 3 * 16

    def parse(self, address, bits):
        self.append_word(address, bits, "Dipole 1")
        self.append_word(address, bits, "Dipole 2")
        self.append_word(address, bits, "Dipole 3")
