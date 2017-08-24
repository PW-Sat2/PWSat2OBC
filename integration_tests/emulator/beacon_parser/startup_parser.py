from parser import CategoryParser


class StartupParser(CategoryParser):
    def __init__(self, reader, store):
        CategoryParser.__init__(self, '01: Startup', reader, store)

    def get_bit_count(self):
        return 9*8

    def parse(self):
        self.append_dword("Boot Counter")
        self.append_byte("Boot Index")
        self.append_dword("Boot Reason")
