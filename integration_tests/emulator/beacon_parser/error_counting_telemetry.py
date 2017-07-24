from parser import Parser


class ErrorCountingTelemetry(Parser):
    def __init__(self, tree_control):
        Parser.__init__(self, tree_control, 'Error Counters')

    def get_bit_count(self):
        return 12*8

    def parse(self, address, bits):

        self.append_byte(address, bits, "Comm")
        self.append_byte(address, bits, "EPS")
        self.append_byte(address, bits, "RTC")
        self.append_byte(address, bits, "IMTQ")
        self.append_byte(address, bits, "N25Q1")
        self.append_byte(address, bits, "N25Q2")
        self.append_byte(address, bits, "N25Q3")
        self.append_byte(address, bits, "N25q TMR")
        self.append_byte(address, bits, "FRAM TMR")
        self.append_byte(address, bits, "Payload")
        self.append_byte(address, bits, "Camera")
        self.append_byte(address, bits, "ExpSuns")
