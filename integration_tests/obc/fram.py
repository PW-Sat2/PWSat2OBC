from .obc_mixin import OBCMixin, command, decode_words

class FRAMMixin(OBCMixin):
    def __init__(self):
        pass

    @command("fram status {0}")
    def fram_status(self, index):
        pass

    @decode_words()
    @command("fram read {0} {1} {2}")
    def fram_read(self, index, address, length):
        pass

    def fram_write(self, index, address, data):
        return self._command("fram write {0} {1} " + " ".join(data), index, address)

