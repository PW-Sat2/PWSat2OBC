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


class NormalBoot:
    def __init__(self):
        pass

    def boot(self, bootloader):
        bootloader.wait_for_prompt()
        bootloader.write('b')


class SelectRunlevel:
    def __init__(self, runlevel):
        self._runlevel = runlevel

    def boot(self, bootloader):
        bootloader.wait_for_prompt()

        bootloader.write('R')
        bootloader.wait_for_prompt(trigger=False, prompt=':')

        bootloader.write(chr(self._runlevel))


class ClearState:
    def __init__(self, ):
        pass

    def boot(self, bootloader):
        bootloader.wait_for_prompt()

        bootloader.write('N')
        bootloader.wait_for_prompt(trigger=False, prompt=':')
        bootloader.write('Y')


class BootHandler:
    def __init__(self, steps):
        self._steps = steps

    def boot(self, bootloader):
        for step in self._steps:
            step.boot(bootloader)
