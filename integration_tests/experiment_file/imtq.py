from parsec import count

from base import pid, int32, label_as, int16

Magnetometer = pid(0x19) >> count(int32, 3)
Magnetometer >>= label_as('Magnetometer')

Dipoles = pid(0x20) >> count(int16, 3)
Dipoles >>= label_as('Dipoles')
