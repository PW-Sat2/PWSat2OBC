from parsec import joint
from parsing import *

AdcsConfiguration = field('Built In Detumbling Disabled', boolean)
AdcsConfiguration >>= label_as('Adcs Configuration')
