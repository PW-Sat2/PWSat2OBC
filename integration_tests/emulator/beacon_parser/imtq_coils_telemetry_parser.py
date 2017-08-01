from parser import Parser


class ImtqCoilsTelemetryParser(Parser):
    def __init__(self, tree_control):
        Parser.__init__(self, tree_control, 'Imtq Coils')

    def get_bit_count(self):
        return 3 * 16

    def parse(self, address, bits):
        self.append_word(address, bits, "Coil Current 1")
        self.append_word(address, bits, "Coil Current 2")
        self.append_word(address, bits, "Coil Current 3")
