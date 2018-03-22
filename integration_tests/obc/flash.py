from .obc_mixin import OBCMixin, command, decode_words, decode_return
import re

class FlashMixin(OBCMixin):
    def __init__(self):
        pass

    def _parse_testflash(result):
        data = {}
        lines = result.splitlines()
        for line in lines:
            value = re.search("Flash ([0-9]+) id valid: ([0-9]+)", line).groups()
            data["Flash {}".format(value[0])] = int(value[1])
        return data


    @decode_return(_parse_testflash)
    @command("testflash")
    def test_flash(self):
        pass
