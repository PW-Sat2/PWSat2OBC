import struct

from telecommand.base import Telecommand, CorrelatedTelecommand
from utils import ensure_byte_list


class DownloadFile(Telecommand):
    def __init__(self, correlation_id, path, seqs):
        self._path = path
        self._correlation_id = correlation_id
        self._seqs = seqs

    def apid(self):
        return 0xAB

    def payload(self):
        seqs_bytes = ensure_byte_list(struct.pack('<' + 'L' * len(self._seqs), *self._seqs))

        return [self._correlation_id, len(self._path)] + list(self._path) + [0x0] + seqs_bytes

    def __repr__(self):
        return "{}, cid={:02d}, {} chunks of '{}'".format(
            super(DownloadFile, self).__repr__(),
            self._correlation_id,
            len(self._seqs), self._path)


class RemoveFile(CorrelatedTelecommand):
    def __init__(self, correlation_id, path):
        super(RemoveFile, self).__init__(correlation_id)
        self._path = path

    def apid(self):
        return 0xAC

    def payload(self):
        return [self._correlation_id, len(self._path)] + list(self._path) + [0x0]

    def __repr__(self):
        return "{}, path={}".format(
            super(RemoveFile, self).__repr__(),
            self._path)


class ListFiles(CorrelatedTelecommand):
    def __init__(self, correlation_id, path):
        super(ListFiles, self).__init__(correlation_id)
        self._path = path

    def apid(self):
        return 0x0F

    def payload(self):
        return [self._correlation_id] + list(self._path) + [0x0]

    def __repr__(self):
        return "{}, path={}".format(
            super(ListFiles, self).__repr__(),
            self._path)


class EraseFlash(CorrelatedTelecommand):
    def __init__(self, correlation_id):
        super(EraseFlash, self).__init__(correlation_id)

    def apid(self):
        return 0x07

    def payload(self):
        return [self._correlation_id]