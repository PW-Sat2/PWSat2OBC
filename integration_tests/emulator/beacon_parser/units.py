from datetime import timedelta


def unit(symbol):
    def wrapper(cls):
        cls.__str__ = lambda self: '{self.converted:.2f} {unit}'.format(self=self, unit=symbol)
        cls.unit = symbol
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
        return '0x{:04X}'.format(self.raw)


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


class BoolType(TelemetryUnit):
    def __init__(self, raw):
        super(BoolType, self).__init__(raw, raw != 0)

    def __str__(self):
        if self.converted:
            return 'True'
        else:
            return 'False'


class BoolTypeNegated(TelemetryUnit):
    def __init__(self, raw):
        super(BoolTypeNegated, self).__init__(raw, raw != 0)

    def __str__(self):
        if self.converted:
            return 'False'
        else:
            return 'True'


class PlainNumber(TelemetryUnit):
    def __init__(self, raw):
        super(PlainNumber, self).__init__(raw, raw)


class EnumValue(TelemetryUnit):
    def __init__(self, enum_type, raw):
        try:
            converted = enum_type(raw).name
        except ValueError:
            converted = 'None'

        super(EnumValue, self).__init__(raw, converted)
        self.enum_type = enum_type
        self.unit = enum_type.__name__

    def __str__(self):
        return self.converted


def enum(enum_type):
    def build(raw):
        return EnumValue(enum_type, raw)

    return build