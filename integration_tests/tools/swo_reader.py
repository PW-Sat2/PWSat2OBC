import serial
import sys

port = serial.Serial(port='COM38', baudrate=900000)


def is_bit_set(value, bit):
    return value & (1 << bit) != 0


def is_bit_clear(value, bit):
    return value & (1 << bit) == 0


while True:
    header = port.read(1)

    pld = port.read(1)
    sys.stdout.write(pld)

    sys.stdout.flush()
