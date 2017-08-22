from parsec import count, joint

from base import pid, label_as, field, to_dict
from parsing import byte, uint16

PayloadWhoAmI = pid(0x30) >> count(byte, 1)
PayloadWhoAmI >>= label_as('Payload Who Am I')

PayloadHousekeeping = pid(0x34) >> joint(
    field('INT 3V3D', uint16),
    field('OBC 3V3D', uint16),
).bind(to_dict)
PayloadHousekeeping >>= label_as('Payload Housekeeping')

PayloadObcTelemetry = pid(0x36) >> count(uint16, 41)
PayloadObcTelemetry >>= label_as('Payload Obc Telemetry')
