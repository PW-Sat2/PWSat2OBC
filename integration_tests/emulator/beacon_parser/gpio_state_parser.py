from parser import Parser


class GpioStateParser(Parser):
    def __init__(self, tree_control):
        Parser.__init__(self, tree_control, 'GPIO')

    def get_bit_count(self):
        return 1

    def parse(self, address, bits):
        self.append_byte(address, bits, "Sail Deployed", 1)

