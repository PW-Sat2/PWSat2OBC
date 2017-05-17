import struct

from response_frames import ResponseFrame, response_frame
from utils import ensure_string


@response_frame(0x04)
class EntryEraseSuccessFrame(ResponseFrame):
    @classmethod
    def matches(cls, payload):
        return len(payload) >= 3 and payload[0:2] == [0, 0xFF]

    def decode(self):
        self.entries = self.payload()[2]


@response_frame(0x04)
class EntryProgramPartWriteSuccess(ResponseFrame):
    @classmethod
    def matches(cls, payload):
        return len(payload) == 8 and payload[0:2] == [1, 0xFF]

    def decode(self):
        (_, _, self.entries, self.offset, self.size) = struct.unpack('<BBBIB', ensure_string(self.payload()))


@response_frame(0x04)
class EntryFinalizeSuccess(ResponseFrame):
    @classmethod
    def matches(cls, payload):
        return len(payload) == 5 and payload[0:2] == [2, 0xFF]

    def decode(self):
        (_, _, self.entries, self.crc) = struct.unpack('<BBBH', ensure_string(self.payload()))
