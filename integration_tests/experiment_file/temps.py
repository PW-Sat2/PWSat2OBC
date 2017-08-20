from experiment_file.base import uint16, count, label_as, pid

AllTemperatures = pid(0x32) >> count(uint16, 9)
AllTemperatures >>= label_as('Temperatures')
