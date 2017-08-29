from parser import CategoryParser


class ErrorCountingTelemetry(CategoryParser):
    def __init__(self, reader, store):
        CategoryParser.__init__(self, '04: Error Counters', reader, store)

    def get_bit_count(self):
        return 12*8

    def parse(self):
        self.append_byte("Comm")
        self.append_byte("EPS")
        self.append_byte("RTC")
        self.append_byte("IMTQ")
        self.append_byte("N25Q1")
        self.append_byte("N25Q2")
        self.append_byte("N25Q3")
        self.append_byte("N25q TMR")
        self.append_byte("FRAM TMR")
        self.append_byte("Payload")
        self.append_byte("Camera")
        self.append_byte("ExpSuns")
