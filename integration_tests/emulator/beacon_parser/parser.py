from utils import bits_to_dword, bits_to_byte, bits_to_word, bits_to_qword


class BitReader:
    def __init__(self, bits):
        self._offset = 0
        self._bits = bits

    def get_converter(self, length):
        if length <= 8:
            return bits_to_byte
        if length <= 16:
            return bits_to_word
        if length <= 32:
            return bits_to_dword

        return bits_to_qword

    def read(self, length):
        if len(self._bits[self._offset:self._offset + length]) < length:
            return None

        converter = self.get_converter(length)
        result = converter(self._bits[self._offset:self._offset + length])

        self._offset += length

        return result

    def read_byte(self):
        return self.read(8)

    def read_word(self):
        return self.read(16)

    def read_dword(self):
        return self.read(32)

    def read_qword(self):
        return self.read(64)

    def offset(self):
        return self._offset


class BeaconStorage:
    def __init__(self):
        self.storage = {}

    def write(self, category, name, value):
        if category not in self.storage.keys():
            self.storage[category] = {}

        self.storage[category][name] = value


class CategoryParser:
    def __init__(self, category, reader, store):
        self._category = category
        self._reader = reader
        self._store = store

    def append(self, name, length):
        value = self._reader.read(length)
        self._store.write(self._category, self._format_name(name, length), value)

    def append_byte(self, name):
        self.append(name, 8)

    def append_word(self, name):
        self.append(name, 16)

    def append_dword(self, name):
        self.append(name, 32)

    def append_qword(self, name):
        self.append(name, 64)

    def _format_name(self, name, length):
        return str(self._reader.offset() - length).rjust(4, '0') + ": " + name
