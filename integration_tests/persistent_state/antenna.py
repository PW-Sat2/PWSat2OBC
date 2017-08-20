from parsec import joint
from parsing import *

AntennaConfiguration = field('Deployment Disabled', boolean)
AntennaConfiguration >>= label_as('Antenna Configuration')
