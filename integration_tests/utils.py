from string import Formatter
from threading import Event
import inspect

def hex_data(data):
    if isinstance(data, basestring):
        data = [ord(c) for c in data]

    return str(['%X' % b for b in data])

def pad_to_multiply(s, base):
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