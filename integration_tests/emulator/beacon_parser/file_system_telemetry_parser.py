from parser import CategoryParser


class FileSystemTelemetryParser(CategoryParser):
    def __init__(self, reader, store):
        CategoryParser.__init__(self, '06: File System', reader, store)

    def get_bit_count(self):
        return 32

    def parse(self):
        self.append_dword("Free Space")

