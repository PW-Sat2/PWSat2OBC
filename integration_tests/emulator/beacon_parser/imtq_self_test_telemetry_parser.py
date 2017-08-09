from parser import Parser


class ImtqSelfTestTelemetryParser(Parser):
    def __init__(self, tree_control):
        Parser.__init__(self, tree_control, 'Imtq Self Test')

    def get_bit_count(self):
        return 8 * 8

    def parse(self, address, bits):
        self.append_byte(address, bits, "Error 1")
        self.append_byte(address, bits, "Error 2")
        self.append_byte(address, bits, "Error 3")
        self.append_byte(address, bits, "Error 4")
        self.append_byte(address, bits, "Error 5")
        self.append_byte(address, bits, "Error 6")
        self.append_byte(address, bits, "Error 7")
        self.append_byte(address, bits, "Error 8")

