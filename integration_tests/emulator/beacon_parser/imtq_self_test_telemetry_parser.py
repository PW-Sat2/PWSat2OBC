from parser import CategoryParser


class ImtqSelfTestTelemetryParser(CategoryParser):
    def __init__(self, reader, store):
        CategoryParser.__init__(self, '23: Imtq Self Test', reader, store)

    def get_bit_count(self):
        return 8 * 8

    def parse(self):
        self.append_byte("Error 1")
        self.append_byte("Error 2")
        self.append_byte("Error 3")
        self.append_byte("Error 4")
        self.append_byte("Error 5")
        self.append_byte("Error 6")
        self.append_byte("Error 7")
        self.append_byte("Error 8")

