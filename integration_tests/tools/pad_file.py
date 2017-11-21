import argparse

import sys

parser = argparse.ArgumentParser()


def int_dec_or_hex(v):
    if v[0:2] == '0x':
        return int(v, 16)

    return int(v)


parser.add_argument("file", help="File that will be padded")
parser.add_argument("size", help="Size of the file with padding", type=int_dec_or_hex)
parser.add_argument("pad", help="Byte used as padding", type=int_dec_or_hex, default=0)

args = parser.parse_args()

with file(args.file, 'r+b') as f:
    f.seek(0, 2)

    pos = f.tell()

    if pos > args.size:
        sys.stderr.write("Unable to pad to size {} bytes as file is already bigger ({} bytes)".format(pos, args.size))
        exit(-1)

    padding_size = args.size - pos
    padding = chr(args.pad) * padding_size

    f.write(padding)
