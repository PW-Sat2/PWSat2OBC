from utils import bits_to_dword, bits_to_byte, bits_to_word, bits_to_qword, call
from bitarray import bitarray


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

    def append(self, name, length, type_converter = None):
        value = self._reader.read(length)
        converted_value = call(type_converter, value, value)
        self._store.write(self._category, self._format_name(name, length), converted_value)

    def append_byte(self, name, type_converter = None):
        self.append(name, 8, type_converter)

    def append_word(self, name, type_converter = None):
        self.append(name, 16, type_converter)

    def append_dword(self, name, type_converter = None):
        self.append(name, 32, type_converter)

    def append_qword(self, name, type_converter = None):
        self.append(name, 64, type_converter)

    def _format_name(self, name, length):
        return str(self._reader.offset() - length).rjust(4, '0') + ": " + name


class BitArrayParser:
    def __init__(self, parsers_container, data_buffer, store):
        self._parsersContainer = parsers_container
        self._buffer = data_buffer
        self._store = store

    def parse(self):
        all_bits = bitarray(endian='little')
        all_bits.frombytes(self._buffer)
        reader = BitReader(all_bits)

        parsers = self._parsersContainer.GetParsers(reader, self._store)
        parsers.reverse()

        while len(parsers) > 0:
            parser = parsers.pop()
            parser.parse()
