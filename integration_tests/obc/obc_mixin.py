from base64 import b64decode
from functools import wraps

from utils import b64pad


class OBCMixin:
    def __init__(self):
        pass

    def _command(self, cmd, *args, **kwargs):
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


def command(fmt):
    def wrap(f):
        @wraps(f)
        def wrapper(self, *args, **kwargs):
            return self._command(fmt, *args, **kwargs)

        return wrapper

    return wrap
