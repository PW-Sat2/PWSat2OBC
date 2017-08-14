from base import pid, label_as, joint, field, uint16, boolean, to_dict

Sail = pid(0x18) >> \
    joint(
        field('Temperature', uint16),
        field('Open', boolean)
    ).bind(to_dict)
Sail >>= label_as('Sail')
