import serial


class SerialPortTerminal:
    def __init__(self, comPort):
        self._serial = serial.Serial(comPort, baudrate=9600, timeout=1, rtscts=False)
        self._serial.rts = False

    def waitForPrompt(self):
        self._serial.write("\n")
        self._serial.flush()
        c = self._serial.read(1)
        while c != '>':
            c = self._serial.read(1)

    def readUntilPrompt(self):
        data = ""
        c = self._serial.read(1)
        while c != '>':
            data += c
            c = self._serial.read()

        return data

    def command(self, cmd):
        self.waitForPrompt()

        self._serial.write(cmd)
        self._serial.write("\n")

        response = self.readUntilPrompt()

        return response.rstrip('\n')

    def reset(self):
        self._serial.rts = True
        self._serial.rts = False

    def close(self):
        self._serial.close()


class OBC:
    def __init__(self, terminal):
        self._terminal = terminal

    def ping(self):
        return self._terminal.command("ping")

    def jump_to_time(self, time):
        self._terminal.command("jumpToTime %d" % time)

    def current_time(self):
        r = self._terminal.command("currentTime")
        return int(r)

    def send_frame(self, data):
        self._terminal.command("sendFrame %s" % data)

    def reset(self):
        self._terminal.reset()

    def close(self):
        self._terminal.close()

