from parser import CategoryParser


class ImtqTemperatureTelemetryParser(CategoryParser):
    def __init__(self, reader, store):
        CategoryParser.__init__(self, '21: Imtq Temperature', reader, store)

    def get_bit_count(self):
        return 3 * 16

    def parse(self):
        self.append_word("Coil Temperature 1")
        self.append_word("Coil Temperature 2")
        self.append_word("Coil Temperature 3")
