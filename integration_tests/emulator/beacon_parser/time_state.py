from parser import Parser


class TimeState(Parser):
    def __init__(self, tree_control):
        Parser.__init__(self, tree_control, 'Time Telemetry')

    def get_bit_count(self):
        return 64 + 32

    def parse(self, address, bits):
        self.append_qword(address, bits, "Mission time")
        self.append_dword(address, bits, "External time")