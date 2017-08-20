from base import pid
from parsing import *
from parsec import joint

Sail = pid(0x18) >> \
    joint(
        field('Temperature', uint16),
        field('Open', boolean)
    ).bind(to_dict)
Sail >>= label_as('Sail')
