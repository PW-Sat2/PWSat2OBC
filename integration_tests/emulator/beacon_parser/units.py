from datetime import timedelta


def unit(symbol):
    def wrapper(cls):
        cls.__str__ = lambda self: '{self.converted:.2f} {unit}'.format(self=self, unit=symbol)
        return cls

    return wrapper


class TelemetryUnit(object):
    def __init__(self, raw, converted):
        self.raw = raw
        self.converted = converted

    def __repr__(self):
        return str(self.raw)


class Hex16(TelemetryUnit):
    def __init__(self, raw):
        super(Hex16, self).__init__(raw, raw)

    def __str__(self):
        return '{:04X}'.format(self.raw)


class TimeFromMilliseconds(TelemetryUnit):
    def __init__(self, raw):
        super(TimeFromMilliseconds, self).__init__(raw, timedelta(milliseconds=raw))

    def __str__(self):
        return '{}'.format(self.converted)


class TimeFromSeconds(TelemetryUnit):
    def __init__(self, raw):
        super(TimeFromSeconds, self).__init__(raw, timedelta(seconds=raw))

    def __str__(self):
        return '{}'.format(self.converted)