from base import uint16, count, label_as, pid

Photodiodes = pid(0x33) >> count(uint16, 4)
Photodiodes >>= label_as('Photodiodes')