from .obc_mixin import OBCMixin, command

class CameraMixin(OBCMixin):
    def __init__(self):
        pass

    def camera_take_jpeg(self, resolution):
        return self._terminal.command_with_read_data("camera jpeg {0}".format(resolution))

    def camera_take_jpeg_to_file(self, resolution, file_name):
        jpeg_data = self._terminal.command_with_read_data("camera jpeg {0}".format(resolution))

        f = open(file_name, 'wb')
        f.write(jpeg_data)
        f.close()

    @command("camera init")
    def camera_sync(self):
        pass


