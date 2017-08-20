from base import pid
from parsing import *
from parsec import joint

Gyro = pid(0x10) >> \
    joint(
        field('X', uint16),
        field('Y', uint16),
        field('Z', uint16),
        field('Temperature', uint16)
    ).bind(to_dict)
Gyro >>= label_as('Gyro')
