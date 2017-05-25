from .boot import Bootloader


class OBCBootloader(Bootloader):
    def __init__(self, port):
        self._port = port

    def wait_for_prompt(self):
        c = self._port.read(1)

        while c != '#':
            c = self._port.read(1)

    def write(self, data):
        self._port.write(data)
