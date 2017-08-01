from parser import Parser


class ImtqHousekeepingTelemetryParser(Parser):
    def __init__(self, tree_control):
        Parser.__init__(self, tree_control, 'Imtq Housekeeping')

    def get_bit_count(self):
        return 5 * 16

    def parse(self, address, bits):
        self.append_word(address, bits, "Digital Voltage")
        self.append_word(address, bits, "Analog Voltage")
        self.append_word(address, bits, "Digital Current")
        self.append_word(address, bits, "Analog Current")
        self.append_word(address, bits, "MCU Temperature")
