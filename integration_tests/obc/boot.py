class BootToIndex:
    def __init__(self, index):
        self._index = index

    def boot(self, write):
        write("B" + chr(self._index))


class BootToUpper:
    def __init__(self):
        pass

    def boot(self, write):
        write('u')
