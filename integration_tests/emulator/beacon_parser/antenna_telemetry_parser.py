from emulator.beacon_parser.units import BoolType, TimeFromSeconds, TimeFromTwoSeconds
from parser import CategoryParser


class AntennaTelemetryParser(CategoryParser):
    def __init__(self, reader, store):
        CategoryParser.__init__(self, '08: Antenna', reader, store)

    def get_bit_count(self):
        return 8 + 8*3 + 8*8

    def parse(self):
        self.append("Antenna 3 Deployed Ch A", 1, value_type=BoolType)
        self.append("Antenna 4 Deployed Ch A", 1, value_type=BoolType)
        self.append("Antenna 2 Deployed Ch A", 1, value_type=BoolType)

        self.append("Antenna 1 Deployed Ch B", 1, value_type=BoolType)
        self.append("Antenna 2 Deployed Ch B", 1, value_type=BoolType)
        self.append("Antenna 1 Deployed Ch A", 1, value_type=BoolType)
        self.append("Antenna 3 Deployed Ch B", 1, value_type=BoolType)
        self.append("Antenna 4 Deployed Ch B", 1, value_type=BoolType)

        self.append("Antenna 1 Activation Count Ch A", 3)
        self.append("Antenna 2 Activation Count Ch A", 3)
        self.append("Antenna 3 Activation Count Ch A", 3)
        self.append("Antenna 4 Activation Count Ch A", 3)

        self.append("Antenna 1 Activation Count Ch B", 3)
        self.append("Antenna 2 Activation Count Ch B", 3)
        self.append("Antenna 3 Activation Count Ch B", 3)
        self.append("Antenna 4 Activation Count Ch B", 3)

        self.append("Antenna 1 Activation Time Ch A", 8, value_type=TimeFromTwoSeconds)
        self.append("Antenna 2 Activation Time Ch A", 8, value_type=TimeFromTwoSeconds)
        self.append("Antenna 3 Activation Time Ch A", 8, value_type=TimeFromTwoSeconds)
        self.append("Antenna 4 Activation Time Ch A", 8, value_type=TimeFromTwoSeconds)

        self.append("Antenna 1 Activation Time Ch B", 8, value_type=TimeFromTwoSeconds)
        self.append("Antenna 2 Activation Time Ch B", 8, value_type=TimeFromTwoSeconds)
        self.append("Antenna 3 Activation Time Ch B", 8, value_type=TimeFromTwoSeconds)
        self.append("Antenna 4 Activation Time Ch B", 8, value_type=TimeFromTwoSeconds)
