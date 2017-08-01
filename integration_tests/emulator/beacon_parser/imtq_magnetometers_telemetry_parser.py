from parser import Parser


class ImtqMagnetometersTelemetryParser(Parser):
    def __init__(self, tree_control):
        Parser.__init__(self, tree_control, 'Imtq Magnetometers')

    def get_bit_count(self):
        return 3 * 32

    def parse(self, address, bits):
        self.append_dword(address, bits, "Magnetometer Measurement 1")
        self.append_dword(address, bits, "Magnetometer Measurement 2")
        self.append_dword(address, bits, "Magnetometer Measurement 3")
