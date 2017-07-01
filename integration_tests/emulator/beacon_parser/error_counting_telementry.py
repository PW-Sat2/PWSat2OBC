from parser import Parser


class ErrorCountingTelementry(Parser):
    def __init__(self, tree_control):
        Parser.__init__(self, tree_control, 'Error Counters')

    def get_bit_count(self):
        return 16*8

    def parse(self, address, bits):

        self.append_byte(address, bits, "")
        self.append_byte(address, bits, "Comm")
        self.append_byte(address, bits, "")
        self.append_byte(address, bits, "")
        self.append_byte(address, bits, "")
        self.append_byte(address, bits, "EPS")
        self.append_byte(address, bits, "IMTQ")
        self.append_byte(address, bits, "RTC")
        self.append_byte(address, bits, "N25Q1")
        self.append_byte(address, bits, "N25Q2")
        self.append_byte(address, bits, "N25Q3")
        self.append_byte(address, bits, "Redundant F25W")

        self.append_byte(address, bits, "")
        self.append_byte(address, bits, "")
        self.append_byte(address, bits, "")
        self.append_byte(address, bits, "")
