from parser import Parser


class ScrubbingTelemetryParser(Parser):
    def __init__(self, tree_control):
        Parser.__init__(self, tree_control, 'Scrubbing State')

    def get_bit_count(self):
        return 32 + 32 + 32

    def parse(self, address, bits):
        self.append_dword(address, bits, "Primary Flash Scrubbing pointer")
        self.append_dword(address, bits, "Secondary Flash Scrubbing pointer")
        self.append_dword(address, bits, "RAM Scrubbing pointer")