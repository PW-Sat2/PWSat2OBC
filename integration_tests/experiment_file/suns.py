from parsec import joint
from base import pid, count
from parsing import *

ExperimentalSunSStatus = joint(
    field('ALS_ACK', uint16),
    field('ALS_Presence', uint16),
    field('ALS_ADC_Valid', uint16),
)
ExperimentalSunSStatus >>= to_dict

SingleALS = count(uint16, 4)

ALSs = count(SingleALS, 3)

Temperatures = joint(
    field('Structure', uint16),
    field('A', uint16),
    field('B', uint16),
    field('C', uint16),
    field('D', uint16),
).bind(to_dict)

ExperimentalSunSPrimary = pid(0x11) \
                          >> joint(
                            field('WhoAmI ', byte),
                            field('Status', ExperimentalSunSStatus),
                            field('VisibleLight', ALSs),
                            field('Temperatures', Temperatures),
                          ).bind(to_dict)
ExperimentalSunSPrimary >>= label_as('ExpSunS.Primary')

ExperimentalSunSSecondary = pid(0x12) \
                            >> joint(
                                field('Parameters',
                                      joint(
                                          field('gain', byte),
                                          field('itime', byte)
                                      ).bind(to_dict)
                                ),
                                field('Infrared', ALSs)
                            ).bind(to_dict)
ExperimentalSunSSecondary >>= label_as('ExpSunS.Secondary')

ReferenceSunS = pid(0x13) \
                >> joint(field('Voltages', count(uint16, 5))).bind(to_dict)
ReferenceSunS >>= label_as('RefSunS')
