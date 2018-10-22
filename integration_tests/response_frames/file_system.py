import struct

from response_frames import response_frame
from response_frames.common import DownlinkApid, GenericSuccessResponseFrame
from utils import ensure_string


@response_frame(DownlinkApid.FileList)
class FileListSuccessFrame(GenericSuccessResponseFrame):
    def decode(self):
        super(FileListSuccessFrame, self).decode()

        self.file_list = []

        raw = self.response
        raw = ensure_string(raw)

        while len(raw) > 0:
            (name, raw) = raw.split('\0', 1)
            print name

            size_bytes = raw[0:4]
            (size,) = struct.unpack('<L', ensure_string(size_bytes))

            raw = raw[4:]

            self.file_list.append((name, size))

    def __repr__(self):
        return "{}, #files: {}".format(
            super(FileListSuccessFrame, self).__repr__(),
            len(self.file_list))
