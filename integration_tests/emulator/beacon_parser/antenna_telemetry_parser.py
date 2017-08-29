from parser import CategoryParser


class AntennaTelemetryParser(CategoryParser):
    def __init__(self, reader, store):
        CategoryParser.__init__(self, '07: Antenna', reader, store)

    def get_bit_count(self):
        return 8 + 8*8 + 8*12

    def parse(self):
        self.append("Antenna 3 Deployed Ch A", 1)
        self.append("Antenna 4 Deployed Ch A", 1)
        self.append("Antenna 2 Deployed Ch A", 1)

        self.append("Antenna 1 Deployed Ch B", 1)
        self.append("Antenna 2 Deployed Ch B", 1)
        self.append("Antenna 1 Deployed Ch A", 1)
        self.append("Antenna 3 Deployed Ch B", 1)
        self.append("Antenna 4 Deployed Ch B", 1)

        self.append_byte("Antenna 1 Activation Count Ch A")
        self.append_byte("Antenna 2 Activation Count Ch A")
        self.append_byte("Antenna 3 Activation Count Ch A")
        self.append_byte("Antenna 4 Activation Count Ch A")

        self.append_byte("Antenna 1 Activation Count Ch B")
        self.append_byte("Antenna 2 Activation Count Ch B")
        self.append_byte("Antenna 3 Activation Count Ch B")
        self.append_byte("Antenna 4 Activation Count Ch B")

        self.append("Antenna 1 Activation Time Ch A", 12)
        self.append("Antenna 2 Activation Time Ch A", 12)
        self.append("Antenna 3 Activation Time Ch A", 12)
        self.append("Antenna 4 Activation Time Ch A", 12)

        self.append("Antenna 1 Activation Time Ch B", 12)
        self.append("Antenna 2 Activation Time Ch B", 12)
        self.append("Antenna 3 Activation Time Ch B", 12)
        self.append("Antenna 4 Activation Time Ch B", 12)
