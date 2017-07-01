from parser import Parser


class ControllerBTelemetryParser(Parser):
    def __init__(self, tree_control):
        Parser.__init__(self, tree_control, 'Controller B')

    def get_bit_count(self):
        return 10 + 10 + 8 + 16 + 32 + 10 + 10

    def parse(self, address, bits):
        self.append_word(address, bits, "BP.Temperature", 10)
        self.append_word(address, bits, "BATC.VOLT_B", 10)
        self.append_byte(address, bits, "Error Code", 8)
        self.append_word(address, bits, "Power Cycle Count", 16)
        self.append_dword(address, bits, "Uptime", 32)
        self.append_word(address, bits, "Temperature", 10)
        self.append_word(address, bits, "Other.Temperature", 10)
