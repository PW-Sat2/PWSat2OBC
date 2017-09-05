from parser import CategoryParser


class EPSControllerBTelemetryParser(CategoryParser):
    def __init__(self, reader, store):
        CategoryParser.__init__(self, '15: Controller B', reader, store)

    def get_bit_count(self):
        return 10 + 10 + 8 + 16 + 32 + 10 + 10 + 10

    def parse(self):
        self.append("BP.Temperature", 10)
        self.append("BATC.VOLT_B", 10)
        self.append("Safety Counter", 8)
        self.append("Power Cycle Count", 16)
        self.append("Uptime", 32)
        self.append("Temperature", 10)
        self.append('SUPP_TEMP', 10)
        self.append("Other.Temperature", 10)
