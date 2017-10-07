from emulator.beacon_parser.units import TelemetryUnit
from parser import CategoryParser


class BootIndex(TelemetryUnit):
    def __init__(self, raw):
        super(BootIndex, self).__init__(raw, raw)

    def __str__(self):
        if self.raw == 0b01000000:
            return 'Upper'

        slots = []

        for i in xrange(0, 6):
            mask = 1 << i
            if (self.raw & mask) == mask:
                slots.append(str(i))

        return 'Slots: ' + ','.join(slots)


class StartupParser(CategoryParser):
    def __init__(self, reader, store):
        CategoryParser.__init__(self, '01: Startup', reader, store)

    def get_bit_count(self):
        return 7 * 8

    def parse(self):
        self.append_dword("Boot Counter")
        self.append_byte("Boot Index", value_type=BootIndex)
        self.append_word("Boot Reason")
