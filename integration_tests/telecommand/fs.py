import struct

from telecommand.base import Telecommand
from utils import ensure_byte_list


class DownloadFile(Telecommand):
    def __init__(self, path, respond_as, seqs):
        self._path = path
        self._respond_as = respond_as
        self._seqs = seqs

    def apid(self):
        return 0xAB

    def payload(self):
        seqs_bytes = ensure_byte_list(struct.pack('<' + 'L' * len(self._seqs), *self._seqs))

        return [self._respond_as, len(self._path)] + list(self._path) + [0x0] + seqs_bytes
