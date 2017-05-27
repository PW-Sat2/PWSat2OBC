from abc import ABCMeta, abstractmethod


class Bootloader:
    __metaclass__ = ABCMeta

    def __init__(self):
        pass

    @abstractmethod
    def wait_for_prompt(self, trigger=None, prompt=None):
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
    def __init__(self, runlevel):
        self._runlevel = runlevel

    def boot(self, next_handler, bootloader):
        bootloader.wait_for_prompt()

        bootloader.write('R')
        bootloader.wait_for_prompt(trigger=False, prompt=':')

        bootloader.write(chr(self._runlevel))

        next_handler.boot(bootloader)


class BootHandlerChain:
    def __init__(self, chain):
        self._chain = chain

    def boot(self, bootloader):
        if len(self._chain) == 1:
            self._chain[0].boot(bootloader)
        else:
            self._chain[0].boot(BootHandlerChain(self._chain[1:]), bootloader)
