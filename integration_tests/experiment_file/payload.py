from parsec import count, joint

from base import pid, label_as, field, to_dict
from parsing import byte, uint16
from parsec import Parser, Value

from emulator.beacon_parser import eps_controller_a_telemetry_parser, eps_controller_b_telemetry_parser, \
    error_counting_telemetry, experiment_telemetry_parser, mcu_temperature_parser
from emulator.beacon_parser.parser import BitReader
from bitarray import bitarray

from math import ceil

PayloadWhoAmI = pid(0x30) >> count(byte, 1)
PayloadWhoAmI >>= label_as('Payload Who Am I')

PayloadHousekeeping = pid(0x34) >> joint(
    field('INT 3V3D', uint16),
    field('OBC 3V3D', uint16),
).bind(to_dict)
PayloadHousekeeping >>= label_as('Payload Housekeeping')

##########################################################################


class PayloadOBCTelemetryParser:
    def __init__(self):
        self.storage = {}

    def write(self, category, name, value):
        if category not in self.storage.keys():
            self.storage[category] = {}

        self.storage[category][name] = value

    def GetParsers(self, reader):
        return [error_counting_telemetry.ErrorCountingTelemetry(reader, self),
                experiment_telemetry_parser.ExperimentTelemetryParser(reader, self),
                mcu_temperature_parser.McuTemperatureParser(reader, self),
                eps_controller_a_telemetry_parser.EPSControllerATelemetryParser(reader, self),
                eps_controller_b_telemetry_parser.EPSControllerBTelemetryParser(reader, self)]

    def GetSize(self):
        parsers = self.GetParsers(None)
        size = 0
        for parser in parsers:
            size = size + parser.get_bit_count()
        return int(ceil(size / 8.0))

@Parser
def PayloadParser(text, index):
    telemetry_parser = PayloadOBCTelemetryParser()
    size = telemetry_parser.GetSize()

    if index + size <= len(text):
        part = text[index: index + size]

        all_bits = bitarray(endian='little')
        all_bits.frombytes(part)
        reader = BitReader(all_bits)
        parsers = telemetry_parser.GetParsers(reader)
        parsers.reverse()

        while len(parsers) > 0:
            parser = parsers.pop()
            parser.parse()

        return Value.success(index + size, telemetry_parser.storage)
    else:
        return Value.failure(index, 'Decode failed')

PayloadObcTelemetry = pid(0x36) >> count(PayloadParser, 1)
PayloadObcTelemetry >>= label_as('Payload Obc Telemetry')
