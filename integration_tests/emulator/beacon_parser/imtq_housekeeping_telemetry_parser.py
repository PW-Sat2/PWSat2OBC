from parser import CategoryParser


class ImtqHousekeepingTelemetryParser(CategoryParser):
    def __init__(self, reader, store):
        CategoryParser.__init__(self, 'Imtq Housekeeping', reader, store)

    def get_bit_count(self):
        return 5 * 16

    def parse(self):
        self.append_word("Digital Voltage")
        self.append_word("Analog Voltage")
        self.append_word("Digital Current")
        self.append_word("Analog Current")
        self.append_word("MCU Temperature")
