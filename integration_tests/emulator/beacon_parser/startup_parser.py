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


class BootReason(TelemetryUnit):
    def __init__(self, raw):
        super(BootReason, self).__init__(raw, raw)

    def __str__(self):
        reason_text = 'Unknown'

        if self.raw & 0b1 == 0b1: # 0bXXXX XXXX XXXX XXX1
            reason_text = 'Power-on Reset'
        elif self.raw & 0b0000000010000011 == 0b0000000000000010:  # 0bXXXX XXXX 0XXX XX10
            reason_text = 'Unregulated Brown-out'
        elif self.raw & 0b0000000000011111 == 0b0000000000000100:  # 0bXXXX XXXX XXX0 0100
            reason_text = 'Regulated Brown-out'
        elif self.raw & 0b0000000000001011 == 0b0000000000001000:  # 0bXXXX XXXX XXXX 1X00
            reason_text = 'External reset'
        elif self.raw & 0b0000000000010011 == 0b0000000000010000:  # 0bXXXX XXXX XXX1 XX00
            reason_text = 'Watchdog reset'
        elif self.raw & 0b0000011111111111 == 0b0000000000100000:  # 0bXXXX X000 0010 0000
            reason_text = 'Lockup reset'
        elif self.raw & 0b0000011111011111 == 0b0000000001000000:  # 0bXXXX X000 01X0 0000
            reason_text = 'System Request reset'
        elif self.raw & 0b0000011110011001 == 0b0000000010000000:  # 0bXXXX X000 1XX0 0XX0
            reason_text = 'Woken up from EM4'
        elif self.raw & 0b0000011110011001 == 0b0000000110000000:  # 0bXXXX X001 1XX0 0XX0
            reason_text = 'Woken up from EM4 by pin'
        elif self.raw & 0b0000011000011111 == 0b0000001000000000:  # 0bXXXX X01X XXX0 0000
            reason_text = 'Brown-out on Analog Power 0'
        elif self.raw & 0b0000011000011111 == 0b0000010000000000:  # 0bXXXX X10X XXX0 0000
            reason_text = 'Brown-out on Analog Power 1'
        elif self.raw & 0b0000100000001001 == 0b0000100000000000:  # 0bXXXX 1XXX XXXX 0XX0
            reason_text = 'Brown-out by Backup BOD on VDD_DREG'
        elif self.raw & 0b0001000000001001 == 0b0001000000000000:  # 0bXXX1 XXXX XXXX 0XX0
            reason_text = 'Brown-out by Backup BOD on BU_VIN'
        elif self.raw & 0b0010000000001001 == 0b0010000000000000:  # 0bXX1X XXXX XXXX 0XX0
            reason_text = 'Brown-out by Backup BOD on unregulated power'
        elif self.raw & 0b0100000000001001 == 0b0100000000000000:  # 0bX1XX XXXX XXXX 0XX0
            reason_text = 'Brown-out by Backup BOD on regulated power'
        elif self.raw & 0b1000000000000001 == 0b1000000000000000:  # 0b1XXX XXXX XXXX XXX0
            reason_text = 'Woken from Backup mode'

        return '{0} - {1}'.format(reason_text, self.raw)


class StartupParser(CategoryParser):
    def __init__(self, reader, store):
        CategoryParser.__init__(self, '01: Startup', reader, store)

    def get_bit_count(self):
        return 7 * 8

    def parse(self):
        self.append_dword("Boot Counter")
        self.append_byte("Boot Index", value_type=BootIndex)
        self.append_word("Boot Reason", value_type=BootReason)
