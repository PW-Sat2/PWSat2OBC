from parser import CategoryParser


class ImtqMagnetometersTelemetryParser(CategoryParser):
    def __init__(self, reader, store):
        CategoryParser.__init__(self, '15: Imtq Magnetometers', reader, store)

    def get_bit_count(self):
        return 3 * 32

    def parse(self):
        self.append_dword("Magnetometer Measurement 1")
        self.append_dword("Magnetometer Measurement 2")
        self.append_dword("Magnetometer Measurement 3")
