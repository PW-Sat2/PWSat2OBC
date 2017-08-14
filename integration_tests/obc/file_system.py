import struct

from .obc_mixin import OBCMixin, command, decode_lines


class FileSystemMixin(OBCMixin):
    def __init__(self):
        pass

    @decode_lines()
    @command("listFiles {0}")
    def list_files(self, path):
        pass

    def write_file(self, path, content):
        return int(self._terminal.command_with_write_data("writeFile {0}".format(path), content))

    def read_file(self, path):
        return self._terminal.command_with_read_data("readFile {0}".format(path))

    @command("sync_fs")
    def sync_fs(self):
        pass

    @command("erase {0}")
    def erase(self, chip_index):
        pass

    @command("rm {0}")
    def remove_file(self, path):
        pass

    def save_all_files(self, obc_path="/", filename_prefix="", local_path=""):
        downloaded_files = []

        files = self.list_files(obc_path)
        for file in files:
            if len(filename_prefix) == 0 or file.startswith(filename_prefix):
                print "Downloading \"" + file + "\""
                content = self.read_file(obc_path + file)
                with open(local_path + file, 'wb') as f:
                    f.write(''.join(content))

                downloaded_files.append(file)

        return downloaded_files

