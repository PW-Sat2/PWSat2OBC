from parser import Parser


class McuTemperatureParser(Parser):
    def __init__(self, tree_control):
        Parser.__init__(self, tree_control, 'MCU')

    def get_bit_count(self):
        return 12

    def parse(self, address, bits):
        self.append_word(address, bits, "Temperature", 12)

