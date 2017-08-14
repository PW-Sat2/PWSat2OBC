class CameraDriver(object):
    def __init__(self, i2cMock, mockpin_nadir, mockpin_wing):
        self.i2cMock = i2cMock
        self.mockpin_nadir = mockpin_nadir
        self.mockpin_wing = mockpin_wing

    def decode_photo(self, local_name):
        result = []

        with open(local_name, 'rb') as f:
            data = f.read()

        rem = data[4:]

        while len(rem) > 0:
            part = rem[0:512 - 6]

            result += part

            rem = rem[512:]

        with open(local_name + '.jpg', 'wb') as f:
            f.write(''.join(result))

    def decode_files(self, files):
        for file_name in files:
            print "Decoding \"" + file_name + "\""
            self.decode_photo(file_name)

    def enable_nadir(self):
        self.i2cMock.gpio_low(self.mockpin_nadir)

    def disable_nadir(self):
        self.i2cMock.gpio_high(self.mockpin_nadir)

    def enable_wing(self):
        self.i2cMock.gpio_low(self.mockpin_wing)

    def disable_wing(self):
        self.i2cMock.gpio_high(self.mockpin_wing)

