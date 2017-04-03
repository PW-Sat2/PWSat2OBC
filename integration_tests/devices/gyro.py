import logging, struct, i2cMock


class Gyro(i2cMock.I2CDevice):
    def __init__(self):
        super(Gyro, self).__init__(0x34)
        self.log = logging.getLogger("Gyro")
        self.data = {}
        self.reset()

    def reset(self):
        self.data = {0x00: 0b01101000,
                     0x15: 0,
                     0x16: 0,
                     0x17: 0,
                     0x1A: 0b101,
                     0x1B: 0,
                     0x1C: 0,
                     0x1D: 0,
                     0x1E: 0,
                     0x1F: 0,
                     0x20: 0,
                     0x21: 0,
                     0x22: 0,
                     0x3E: 0}

    def set(self, x, y, z, temp):
        self.data[0x1B] = ord(list(struct.pack('>h', temp))[0])
        self.data[0x1C] = ord(list(struct.pack('>h', temp))[1])

        self.data[0x1D] = ord(list(struct.pack('>h', x))[0])
        self.data[0x1E] = ord(list(struct.pack('>h', x))[1])

        self.data[0x1F] = ord(list(struct.pack('>h', y))[0])
        self.data[0x20] = ord(list(struct.pack('>h', y))[1])

        self.data[0x21] = ord(list(struct.pack('>h', z))[0])
        self.data[0x22] = ord(list(struct.pack('>h', z))[1])

    # --- Commands ---

    def write_to_register(self, register, value):
        self.log.info("Write to %d" % register)
        for i in value:
            if register in self.data:
                self.data[register] = i
                register += 1
            else:
                self.log.info("Write to wrong register")

    def read_from_register(self, register):
        self.log.info("Read from %d" % register)

        response = []
        while register in self.data:
            response.append(self.data[register])
            register += 1

        return response

    def handle_command(self, cmd, data):
        if len(data) == 0:
            return self.read_from_register(cmd)
        else:
            self.write_to_register(cmd, data)

    @i2cMock.command([])
    def cmd(self, *data):
        response = self.handle_command(data[0], data[1:])

        if len(data) == 2 and data[0] == 0x3E and data[1] == (1 << 7):
            self.reset()

        return response

