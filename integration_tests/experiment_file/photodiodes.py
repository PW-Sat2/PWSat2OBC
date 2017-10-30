from base import uint16, count, label_as, pid, to_dict, field, joint

Photodiodes = pid(0x33) >> joint(
    field('Xn', uint16),
    field('Xp', uint16),
    field('Yn', uint16),
    field('Yp', uint16),
).bind(to_dict)
Photodiodes >>= label_as('Photodiodes')
