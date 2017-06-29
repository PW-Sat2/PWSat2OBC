import serial


class Pins:
    RESET = object()

    def __init__(self, com_port):
        self._com_port = com_port

        self._port = None
        while self._port is None:
            try:
                self._port = serial.Serial(self._com_port, rtscts=False, dsrdtr=False, baudrate=9600)
            except serial.SerialException as e:
                if not e.args[0].find("WindowsError(5") > -1:
                    raise

    def high(self, pin):
        if pin == self.RESET:
            self._port.rts = False

    def low(self, pin):
        if pin == self.RESET:
            self._port.rts = True

    def close(self):
        self._port.close()
