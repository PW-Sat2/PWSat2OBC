from base import *

AntennaConfiguration = joint(
        field('Deployment Disabled', boolean)
    ).bind(to_dict)
AntennaConfiguration >>= label_as('Antenna Configuration')
