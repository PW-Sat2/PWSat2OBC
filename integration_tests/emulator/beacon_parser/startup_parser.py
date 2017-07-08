from parser import Parser

class StartupParser(Parser):
    def __init__(self, tree_control):
        Parser.__init__(self, tree_control, 'Startup')

    def get_bit_count(self):
        return 9*8

    def parse(self,  address, bits):
        self.append_dword(address, bits, "Boot Counter")
        self.append_byte(address, bits, "Boot Index")
        self.append_dword(address, bits, "Boot Reason")
