from parser import Parser


class FileSystemTelemetryParser(Parser):
    def __init__(self, tree_control):
        Parser.__init__(self, tree_control, 'File System')

    def get_bit_count(self):
        return 32

    def parse(self, address, bits):
        self.append_dword(address, bits, "Free Space")

