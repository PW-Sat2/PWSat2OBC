from parsec import joint
from parsing import *

CounterConfig = joint(
    field('Limit', byte),
    field('Increment', byte),
    field('Decrement', byte),
    field('Zero', byte),
    ).bind(to_dict)
CounterConfig >>= label_as('Counter Config')

ErrorCounters = joint(
    field('Comm', CounterConfig),
    field('Eps', CounterConfig),
    field('RTC', CounterConfig),
    field('Imtq', CounterConfig),
    field('N25q Flash 1', CounterConfig),
    field('N25q Flash 2', CounterConfig),
    field('N25q Flash 3', CounterConfig),
    field('N25q TMR', CounterConfig),
    field('FRAM TMR', CounterConfig),
    field('Payload', CounterConfig),
    field('Camera', CounterConfig),
    field('Suns', CounterConfig),
    field('Antenna Primary', CounterConfig),
    field('Antenna Backup', CounterConfig)
    ).bind(to_dict)
ErrorCounters >>= label_as('Error Counters')
