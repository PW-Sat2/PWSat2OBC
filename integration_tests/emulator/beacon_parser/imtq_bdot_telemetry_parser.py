from parser import Parser


class ImtqBDotTelemetryParser(Parser):
    def __init__(self, tree_control):
        Parser.__init__(self, tree_control, 'Imtq BDot')

    def get_bit_count(self):
        return 3 * 32

    def parse(self, address, bits):
        self.append_dword(address, bits, "BDot 1")
        self.append_dword(address, bits, "BDot 2")
        self.append_dword(address, bits, "BDot 3")
