from parsec import joint
from parsing import *

BootState = field('Boot Counter', uint32)
BootState >>= label_as('Boot State')
