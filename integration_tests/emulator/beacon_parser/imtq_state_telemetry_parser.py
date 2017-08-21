from parser import CategoryParser


class ImtqStateTelemetryParser(CategoryParser):
    def __init__(self, reader, store):
        CategoryParser.__init__(self, 'Imtq State', reader, store)

    def get_bit_count(self):
        return 8 + 2 + 2 + 1 + 32

    def parse(self):
        self.append("Status", 8)
        self.append("Mode", 2)
        self.append("Error during previous iteration", 8)
        self.append("Configuration changed", 1)
        self.append("Uptime", 32)
