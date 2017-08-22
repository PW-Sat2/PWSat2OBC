from parser import CategoryParser


class ImtqDipoleTelemetryParser(CategoryParser):
    def __init__(self, reader, store):
        CategoryParser.__init__(self, 'Imtq Dipole', reader, store)

    def get_bit_count(self):
        return 3 * 16

    def parse(self):
        self.append_word("Dipole 1")
        self.append_word("Dipole 2")
        self.append_word("Dipole 3")
