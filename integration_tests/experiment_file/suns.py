from base import pid, bytes_block, label_as

ExperimentalSunSPrimary = pid(0x11) >> bytes_block(39)
ExperimentalSunSPrimary >>= label_as('ExpSunS.Primary')

ExperimentalSunSSecondary = pid(0x12) >> bytes_block(26)
ExperimentalSunSSecondary >>= label_as('ExpSunS.Secondary')

ReferenceSunS = pid(0x13) >> bytes_block(10)
ReferenceSunS >>= label_as('RefSunS')