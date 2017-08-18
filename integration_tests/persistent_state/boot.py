from base import *

BootState = joint(
        field('Boot Counter', uint32)
    ).bind(to_dict)
BootState >>= label_as('Boot State')
