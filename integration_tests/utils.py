from string import Formatter
from threading import Event
import inspect

import time


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
