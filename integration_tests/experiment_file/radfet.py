from parsec import joint
from base import pid, count
from parsing import *

RadFET = pid(0x35) >> joint(
    field('Status', byte),
    field('Temperature', uint32),
    field('Voltages', count(uint32, 3))
).bind(to_dict)
RadFET >>= label_as('RadFET')