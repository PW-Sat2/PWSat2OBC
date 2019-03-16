import struct

from response_frames import ResponseFrame, response_frame
from utils import ensure_string
from telecommand.program_upload import WriteProgramPart


@response_frame(0x04)
class EntryEraseSuccessFrame(ResponseFrame):
    @classmethod
    def matches(cls, payload):
        return len(payload) >= 3 and payload[0:2] == [0, 0]

    def decode(self):
        self.entries = self.payload()[2]


@response_frame(0x04)
class EntryProgramPartWriteSuccess(ResponseFrame):
    @classmethod
    def matches(cls, payload):
        return len(payload) == 8 and payload[0:2] == [1, 0]

    def decode(self):
        (_, _, self.entries, self.offset, self.size) = struct.unpack('<BBBIB', ensure_string(self.payload()))

    def __repr__(self):
        chunk = self.offset * 1.0 / WriteProgramPart.MAX_PART_SIZE
        if chunk.is_integer():
            chunk = int(chunk)
        return "Upload[{}]".format(chunk)


@response_frame(0x04)
class EntryFinalizeSuccess(ResponseFrame):
    @classmethod
    def matches(cls, payload):
        return len(payload) == 5 and payload[0:2] == [2, 0]

    def decode(self):
        (_, _, self.entries, self.crc) = struct.unpack('<BBBH', ensure_string(self.payload()))


@response_frame(0x1D)
class CopyBootSlots(ResponseFrame):
    @classmethod
    def matches(cls, payload):
        return True


@response_frame(0x04)
class EntryEraseError(ResponseFrame):
    @classmethod
    def matches(cls, payload):
        return len(payload) == 8 and payload[0:2] == [0, 1]

    def decode(self):
        (_, _, self.code, self.entry, self.offset) = struct.unpack('<BBBBI', ensure_string(self.payload()))

    def __repr__(self):
        return "{}: Seq={:02d} Code={} Entry={} @ 0x{:X}".format(self.__class__.__name__, self._seq, self.code, self.entry, self.offset)


@response_frame(0x04)
class EntryEraseMalformedError(ResponseFrame):
    @classmethod
    def matches(cls, payload):
        return len(payload) == 3 and payload[0:2] == [0, 1, 10]


@response_frame(0x04)
class EntryProgramPartWriteError(ResponseFrame):
    @classmethod
    def matches(cls, payload):
        return len(payload) == 8 and payload[0:2] == [1, 1]

    def decode(self):
        (_, _, self.code, self.entry, self.offset) = struct.unpack('<BBBBI', ensure_string(self.payload()))

    def __repr__(self):
        return "{}: Seq={:02d} Code={} Entry={} @ 0x{:X}".format(self.__class__.__name__, self._seq, self.code, self.entry, self.offset)

@response_frame(0x04)
class EntryProgramPartWriteMalformedError(ResponseFrame):
    @classmethod
    def matches(cls, payload):
        return len(payload) == 3 and payload[0:2] == [1, 1, 10]

@response_frame(0x04)
class EntryFinalizeError(ResponseFrame):
    @classmethod
    def matches(cls, payload):
        return len(payload) == 4 and payload[0:2] == [2, 1]

    def decode(self):
        (_, _, self.code, self.entry) = struct.unpack('<BBBB', ensure_string(self.payload()))

    def __repr__(self):
        return "{}: Seq={:02d} Code={} Entry={}".format(self.__class__.__name__, self._seq, self.code, self.entry)

@response_frame(0x04)
class EntryFinalizeCRCError(ResponseFrame):
    @classmethod
    def matches(cls, payload):
        return len(payload) == 6 and payload[0:2] == [2, 20]

    def decode(self):
        (_, _, self.entry, self.crc) = struct.unpack('<BBBH', ensure_string(self.payload()))

    def __repr__(self):
        return "{}: Seq={:02d} Entry={} CRC=0x{:X}".format(self.__class__.__name__, self._seq, self.entry, self.crc)

@response_frame(0x04)
class EntryFinalizeMalformedError(ResponseFrame):
    @classmethod
    def matches(cls, payload):
        return len(payload) == 3 and payload[0:2] == [2, 1, 10]
