from parser import Parser


class ScrubbingTelemetryParser(Parser):
    def __init__(self, tree_control):
        Parser.__init__(self, tree_control, 'Scrubbing State')

    def get_bit_count(self):
        return 3 + 3 + 32

    def parse(self, address, bits):
        self.append_byte(address, bits, "Primary Flash Scrubbing pointer", 3)
        self.append_byte(address, bits, "Secondary Flash Scrubbing pointer", 3)
        self.append_dword(address, bits, "RAM Scrubbing pointer")