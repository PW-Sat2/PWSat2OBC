import struct

from telecommand.base import Telecommand
from utils import ensure_byte_list


class DownloadFile(Telecommand):
    def __init__(self, path, correlation_id, seqs):
        self._path = path
        self._correlation_id = correlation_id
        self._seqs = seqs

    def apid(self):
        return 0xAB

    def payload(self):
        seqs_bytes = ensure_byte_list(struct.pack('<' + 'L' * len(self._seqs), *self._seqs))

        return [self._correlation_id, len(self._path)] + list(self._path) + [0x0] + seqs_bytes


class RemoveFile(Telecommand):
    def __init__(self, correlation_id, path):
        self._path = path
        self._correlation_id = correlation_id

    def apid(self):
        return 0xAC

    def payload(self):
        return [self._correlation_id, len(self._path)] + list(self._path) + [0x0]


class ListFiles(Telecommand):
    def __init__(self, correlation_id, path):
        self._path = path
        self._correlation_id = correlation_id

    def apid(self):
        return 0x0F

    def payload(self):
        return [self._correlation_id] + list(self._path) + [0x0]