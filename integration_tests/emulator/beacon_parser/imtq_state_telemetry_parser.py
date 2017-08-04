from parser import Parser


class ImtqStateTelemetryParser(Parser):
    def __init__(self, tree_control):
        Parser.__init__(self, tree_control, 'Imtq State')

    def get_bit_count(self):
        return 8 + 2 + 2 + 1 + 32

    def parse(self, address, bits):
        self.append_byte(address, bits, "Status")
        self.append_byte(address, bits, "Mode", 2)
        self.append_byte(address, bits, "Error during previous iteration", 8)
        self.append_byte(address, bits, "Configuration changed", 1)
        self.append_dword(address, bits, "Uptime")
