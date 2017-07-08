from parser import Parser


class AntennaTelemetryParser(Parser):
    def __init__(self, tree_control):
        Parser.__init__(self, tree_control, 'Antenna')

    def get_bit_count(self):
        return 8 + 8*8 + 8*12

    def parse(self, address, bits):
        self.append_byte(address, bits, "Antenna 1 Deployed Ch A", 1)
        self.append_byte(address, bits, "Antenna 2 Deployed Ch A", 1)
        self.append_byte(address, bits, "Antenna 3 Deployed Ch A", 1)
        self.append_byte(address, bits, "Antenna 4 Deployed Ch A", 1)

        self.append_byte(address, bits, "Antenna 1 Deployed Ch B", 1)
        self.append_byte(address, bits, "Antenna 2 Deployed Ch B", 1)
        self.append_byte(address, bits, "Antenna 3 Deployed Ch B", 1)
        self.append_byte(address, bits, "Antenna 4 Deployed Ch B", 1)

        self.append_byte(address, bits, "Antenna 1 Activation Count Ch A")
        self.append_byte(address, bits, "Antenna 2 Activation Count Ch A")
        self.append_byte(address, bits, "Antenna 3 Activation Count Ch A")
        self.append_byte(address, bits, "Antenna 4 Activation Count Ch A")

        self.append_byte(address, bits, "Antenna 1 Activation Count Ch B")
        self.append_byte(address, bits, "Antenna 2 Activation Count Ch B")
        self.append_byte(address, bits, "Antenna 3 Activation Count Ch B")
        self.append_byte(address, bits, "Antenna 4 Activation Count Ch B")

        self.append_word(address, bits, "Antenna 1 Activation Time Ch A", 12)
        self.append_word(address, bits, "Antenna 2 Activation Time Ch A", 12)
        self.append_word(address, bits, "Antenna 3 Activation Time Ch A", 12)
        self.append_word(address, bits, "Antenna 4 Activation Time Ch A", 12)

        self.append_word(address, bits, "Antenna 1 Activation Time Ch B", 12)
        self.append_word(address, bits, "Antenna 2 Activation Time Ch B", 12)
        self.append_word(address, bits, "Antenna 3 Activation Time Ch B", 12)
        self.append_word(address, bits, "Antenna 4 Activation Time Ch B", 12)
