from abc import ABCMeta, abstractmethod


class Bootloader:
    __metaclass__ = ABCMeta

    def __init__(self):
        pass

    @abstractmethod
    def wait_for_prompt(self):
        raise NotImplemented()

    @abstractmethod
    def write(self, data):
        raise NotImplemented()


class BootToIndex:
    def __init__(self, index):
        self._index = index

    def boot(self, bootloader):
        bootloader.wait_for_prompt()
        bootloader.write("B" + chr(self._index))


class BootToUpper:
    def __init__(self):
        pass

    def boot(self, bootloader):
        bootloader.wait_for_prompt()
        bootloader.write('u')


class SelectRunlevel:
    def __init__(self, next_handler):
        self._next_handler = next_handler

    def boot(self, bootloader):
        bootloader.wait_for_prompt()
        # select runlevel
        self._next_handler.boot(bootloader)
