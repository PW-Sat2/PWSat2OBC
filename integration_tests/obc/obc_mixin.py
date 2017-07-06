from base64 import b64decode
from functools import wraps
import re

from utils import b64pad


class OBCMixin:
    def __init__(self):
        pass

    def _command(self, cmd, *args, **kwargs):
        pass

    def _command_no_wait(self, cmd, *args, **kwargs):
        pass


def decode_return(decoder):
    def wrap(f):
        @wraps(f)
        def wrapper(self, *args, **kwargs):
            result = f(self, *args, **kwargs)
            return decoder(result)

        return wrapper

    return wrap


def decode_base64():
    def p(s):
        return b64decode(b64pad(s))

    return decode_return(p)


def decode_words():
    def p(s):
        return s.split()

    return decode_return(p)


def decode_lines():
    def p(s):
        return s.split('\n')

    return decode_return(p)


def decode_bool():
    def p(s):
        return s == '1'

    return decode_return(p)


def decode_multiline_dictionary():
    def p(s):
        if "failed" in s:
            return None

        # Pattern: 'Label: <value> [optional uninteresting text]'. Output: (label,value).
        pattern = re.compile("([^:]+):\s+(\d+).*")
        result = []
        lines = s.split('\n')
        for line in lines:
            match = pattern.match(line)
            if match:
                result.append((match.group(1), int(match.group(2))))

        return dict(result)

    return decode_return(p)


def command(fmt):
    def wrap(f):
        @wraps(f)
        def wrapper(self, *args, **kwargs):
            return self._command(fmt, *args, **kwargs)

        return wrapper

    return wrap
