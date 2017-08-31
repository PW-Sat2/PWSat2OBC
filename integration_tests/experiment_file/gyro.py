from base import pid
from parsing import *
from parsec import joint

Gyro = pid(0x10) >> \
    joint(
        field('X', int16),
        field('Y', int16),
        field('Z', int16),
        field('Temperature', int16)
    ).bind(to_dict)
Gyro >>= label_as('Gyro')
