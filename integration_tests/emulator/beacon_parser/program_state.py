from parser import CategoryParser


class ProgramStateParser(CategoryParser):
    def __init__(self, reader, store):
        CategoryParser.__init__(self, 'Program State', reader, store)

    def get_bit_count(self):
        return 2*8

    def parse(self):
        self.append_word("Program CRC")
