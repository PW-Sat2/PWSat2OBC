from base import uint16, count, label_as, pid, to_dict, field, joint

AllTemperatures = pid(0x32) >> joint(
    field('Supply', uint16),
    field('Xp', uint16),
    field('Xn', uint16),
    field('Yp', uint16),
    field('Yn', uint16),
    field('SADS', uint16),
    field('Sail', uint16),
    field('Camera Nadir', uint16),
    field('Camera Wing', uint16),
).bind(to_dict)
AllTemperatures >>= label_as('Temperatures')

SupplySADSTemperatures = pid(0x17) >> joint(
    field('Supply', uint16),
    field('SADS', uint16)
).bind(to_dict)
SupplySADSTemperatures >>= label_as('Temperatures Supply & SADS')
