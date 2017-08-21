from parser import CategoryParser


class ImtqBDotTelemetryParser(CategoryParser):
    def __init__(self, reader, store):
        CategoryParser.__init__(self, 'Imtq BDot', reader, store)

    def get_bit_count(self):
        return 3 * 32

    def parse(self):
        self.append_dword("BDot 1")
        self.append_dword("BDot 2")
        self.append_dword("BDot 3")
