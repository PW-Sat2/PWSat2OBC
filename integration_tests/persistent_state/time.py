from base import *

time = packed('<Q').parsecmap(lambda x: timedelta(milliseconds=x))

TimeCorrection = joint(
    field('Internal Clock Weight', int16),
    field('External Clock Weight', int16)
    ).bind(to_dict)

TimeCorrection >>= label_as('Time Correction')

MissionTime = joint(
    field('Internal Time', time),
    field('External Time', time)
    ).bind(to_dict)

MissionTime >>= label_as('Mission Time')
