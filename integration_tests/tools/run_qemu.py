import sys
import subprocess

qemu = sys.argv[1]
test_binary = sys.argv[2]

test_args = ','.join(map(lambda v: "arg={}".format(v), sys.argv[3:]))

qemu_args = [
    qemu,
    '-board', 'generic',
    '-mcu', 'EFM32GG990F1024_BIG',
    '-nographic',
    '-monitor', 'null',
    '-image', test_binary,
    '-semihosting-config', 'arg=tests,{}'.format(test_args)
]

qemu_args_s = ' '.join(map(lambda v: '"{}"'.format(v), qemu_args))

subprocess.call(qemu_args_s)
