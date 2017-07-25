import struct

from .obc_mixin import OBCMixin, command, decode_lines

class CameraMixin(OBCMixin):
    def __init__(self):
        pass

    def takeJpeg(self, resolution):
        return self._terminal.command_with_read_data("camera jpeg {0}".format(resolution))

    def takeJpegToFile(self, resolution, file):
        jpegdata = self._terminal.command_with_read_data("camera jpeg {0}".format(resolution))

        f = open(file, 'wb')
        f.write(jpegdata)
        f.close()

    @command("camera init")
    def sync(self):
        pass