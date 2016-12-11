from .obc_mixin import OBCMixin, command, decode_lines


class FileSystemMixin(OBCMixin):
    def __init__(self):
        pass

    @decode_lines()
    @command("listFiles {0}")
    def list_files(self, path):
        pass

    @command("writeFile {0} {1}")
    def write_file(self, path, content):
        pass

    @command("readFile {0}")
    def read_file(self, path):
        pass

    @command("sync_fs")
    def sync_fs(self):
        pass