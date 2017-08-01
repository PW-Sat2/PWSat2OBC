from parser import Parser


class ImtqCoilActiveTelemetryParser(Parser):
    def __init__(self, tree_control):
        Parser.__init__(self, tree_control, 'Imtq Coils Active')

    def get_bit_count(self):
        return 1

    def parse(self, address, bits):
        self.append_byte(address, bits, "Coils active during measurement", 1)
