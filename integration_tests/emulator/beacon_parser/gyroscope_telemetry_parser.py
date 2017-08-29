from parser import CategoryParser


class GyroscopeTelemetryParser(CategoryParser):
    def __init__(self, reader, store):
        CategoryParser.__init__(self, '09: Gyroscope', reader, store)

    def get_bit_count(self):
        return 4 * 16

    def parse(self):
        self.append_word("X measurement")
        self.append_word("Y measurement")
        self.append_word("Z measurement")
        self.append_word("Temperature")

