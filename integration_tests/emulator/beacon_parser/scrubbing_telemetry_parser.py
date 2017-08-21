from parser import CategoryParser


class ScrubbingTelemetryParser(CategoryParser):
    def __init__(self, reader, store):
        CategoryParser.__init__(self, 'Scrubbing State', reader, store)

    def get_bit_count(self):
        return 3 + 3 + 32

    def parse(self):
        self.append("Primary Flash Scrubbing pointer", 3)
        self.append("Secondary Flash Scrubbing pointer", 3)
        self.append_dword("RAM Scrubbing pointer")