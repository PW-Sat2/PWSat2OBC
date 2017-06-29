import logging, struct, i2cMock


class Gyro(i2cMock.I2CDevice):
    def __init__(self):
        super(Gyro, self).__init__(0x68, "Gyro")
        self.data = {}
        self.reset()
        self.initialised = False

    def reset(self):
        self.initialised = False
        self.data = {0x00: 0b01101000,
                     0x15: 0,
                     0x16: 0,
                     0x17: 0,
                     0x1A: 0,
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

    def update(self):
        # check for hardware reset
        if self.data[0x3E] >> 7:
            self.log.info("Hardware reset!")
            self.reset()

        # check if configuration of ITG-3200 is correct and if so mark new data
        self.data[0x1A] = 0
        self.initialised = False
        if self.data[0x16] >> 3 != 0b11:
            self.log.info("Config invalid - wrong FS_SEL!")
            return

        if self.data[0x16] & 0b111 == 7:
            self.log.info("Config invalid - wrong DLPF_CFG!")
            return

        if (self.data[0x17] & 0b10111111) != 0b101:
            self.log.info("Config invalid - wrong Interrupt configuration!")
            return

        self.log.info("Config accepted!")
        self.data[0x1A] = 0b101
        self.initialised = True

    # --- Commands ---
    def write_to_register(self, register, value):
        self.log.info("Write to %d" % register)
        for i in value:
            if register in self.data:
                self.data[register] = i
                register += 1
            else:
                self.log.info("Write to wrong register")
        self.update()

    def read_from_register(self, register):
        self.log.info("Read from %d" % register)

        response = []
        while register in self.data:
            response.append(self.data[register])

            if register == 0x1A and self.data[register] == 0b101:
                self.data[register] = 1

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
        return response

