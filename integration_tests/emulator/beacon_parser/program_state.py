from parser import Parser


class ProgramStateParser(Parser):
    def __init__(self, tree_control):
        Parser.__init__(self, tree_control, 'Program State')

    def get_bit_count(self):
        return 2*8

    def parse(self, address, bits):
        self.append_word(address, bits, "Program CRC")
