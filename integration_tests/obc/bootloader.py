from .boot import Bootloader


class OBCBootloader(Bootloader):
    def __init__(self, port):
        self._port = port

    def wait_for_prompt(self, prompt='#', trigger=True):
        if trigger:
            self._port.write('\n')

        c = self._port.read(1)

        while c != prompt:
            c = self._port.read(1)

    def write(self, data):
        self._port.write(data)
