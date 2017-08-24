from parser import CategoryParser


class TimeState(CategoryParser):
    def __init__(self, reader, store):
        CategoryParser.__init__(self, '03: Time Telemetry', reader, store)

    def get_bit_count(self):
        return 64 + 32

    def parse(self):
        self.append_qword("Mission time")
        self.append_dword("External time")