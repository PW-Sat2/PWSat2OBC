import time
import struct
from string import Formatter
from threading import Event, Timer, Thread
from bitarray import bitarray


def hex_data(data):
    if data is None:
        return []

    if isinstance(data, basestring):
        data = [ord(c) for c in data]

    return str(['0x%X' % b for b in data])

def pad_to_multiply(s, base, char):
    l = base - len(s) % base
    if l < base:
        padding = char * l
    else:
        padding = ''
    return s + padding

def b64pad(s):
    return pad_to_multiply(s, 3, '=')

def lower_byte(value):
    return value & 0xff

def higher_byte(value):
    return (value >> 8) & 0xff

def bitlist_to_byte(bitlist):
    out = 0
    for bit in reversed(bitlist):
        out = (out << 1) | bit

    return out

def pad_bits(bits):
    #while len(bits) % 8 != 0:
    #        bits.insert(0, 0)

    return bits

def bits_to_byte(bits):
    return struct.unpack('<B', bytearray(pad_bits(bits).tobytes()))[0]

def bytes_to_qword(bytes):
    return struct.unpack("<Q", bytearray(bytes))[0]

def bits_to_qword(bits):
    return bytes_to_qword(pad_bits(bits).tobytes())

def bytes_to_dword(bytes):
    return struct.unpack("<I", bytearray(bytes))[0]

def bits_to_dword(bits):
    return bytes_to_dword(pad_bits(bits).tobytes())

def bytes_to_word(bytes):
    return struct.unpack("<H", bytearray(bytes))[0]

def bits_to_word(bits):
    return bytes_to_word(pad_bits(bits).tobytes())

class ExtendableFormatter(Formatter):
    _converters = {}

    def convert_field(self, value, conversion):
        if conversion in self._converters:
            return self._converters[conversion](value)

        return super(ExtendableFormatter, self).convert_field(value, conversion)

    def register_conversion(self, name, func):
        self._converters[name] = func

def call(method, default, *args):
    if method:
        result = method(*args)
        if not result is None:
            return result

    return default

class TestEvent():
    def __init__(self):
        self.flag = Event()

    def set(self, *args):
        self.flag.set()

    def reset(self, *args):
        self.flag.clear();

    def wait_for_change(self, timeout = None):
        return self.flag.wait(timeout)


def ensure_byte_list(value):
    if type(value) is str:
        return ensure_byte_list(list(value))

    if type(value) is tuple:
        return ensure_byte_list(list(value))

    if type(value) is not list:
        return ensure_byte_list([value])

    def ensure_single(v):
        if type(v) is int:
            return v

        if type(v) is str:
            return ord(v)

        return v

    return map(ensure_single, value)


def ensure_string(value):
    if type(value) is str:
        return value

    if type(value) is tuple:
        return ensure_string(list(value))

    if type(value) is not list:
        return ensure_string(list(value))

    def ensure_single(v):
        if type(v) is int:
            return chr(v)

        if type(v) is str:
            return v

        return v

    return ''.join(map(ensure_single, value))


def busy_wait(condition, projection=None, delay=None, timeout=None):
    end_at = None if timeout is None else time.time() + timeout

    while not condition():
        if delay is not None:
            time.sleep(delay)

        if end_at is not None and time.time() > end_at:
            raise Exception('Waiting has timed out')

    if projection is not None:
        return projection
    else:
        return None


class CompareAsDict:
    def __eq__(self, other):
        return self.__dict__ == other.__dict__


class Lazy:
    def __init__(self, create_value):
        self._create_value = create_value
        self._created = False
        self._value = None

    def __call__(self):
        if not self._created:
            self._value = self._create_value()
            self._created = True

        return self._value

class RepeatedTimer:
    def __init__(self, interval, callback):
        self._interval = interval
        self._callback = callback

        self._start = Event()
        self._started = Event()

        self._stop = Event()
        self._stopped = Event()
        self._stopped.set()

        self._thread = Thread(target=self._run)
        self._thread.daemon = True
        self._thread.start()

    def start(self):
        if self._started.wait(0):
            return

        self._stop.clear()
        self._stopped.clear()

        self._started.clear()
        self._start.set()
        self._started.wait()

    def stop(self):
        if self._stopped.wait(0):
            return

        self._start.clear()
        self._stop.set()
        self._stopped.wait()

    def _run(self):
        while True:
            self._start.wait()
            self._started.set()

            while True:
                self._callback()
                if self._stop.wait(self._interval):
                    break

            self._stopped.set()

