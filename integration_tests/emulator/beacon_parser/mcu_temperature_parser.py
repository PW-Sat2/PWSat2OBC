from parser import CategoryParser


class McuTemperatureParser(CategoryParser):
    def __init__(self, reader, store):
        CategoryParser.__init__(self, 'MCU', reader, store)

    def get_bit_count(self):
        return 12

    def parse(self):
        self.append("Temperature", 12)

