from base import pid, bytes_block, label_as

Gyro = pid(0x10) >> bytes_block(8)
Gyro >>= label_as('Gyro')
