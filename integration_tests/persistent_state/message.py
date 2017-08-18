from base import *

minutes = packed('<B').parsecmap(lambda x: timedelta(minutes=x))
array200 = packed('<200s')

Message = joint(
    field('Interval', minutes),
    field('RepeatCount', byte),
    field('Message', array200)
    ).bind(to_dict)
Message >>= label_as('Periodic Message')
