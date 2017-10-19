def encode(data):
    result = []

    for field in data:
        (bit_size, value) = field

        value_bits = [(value & (1 << bit)) > 0 for bit in xrange(0, bit_size)]

        result += value_bits

    print 'Total bits = {}'.format(len(result))

    result = [result[i:i + 8] for i in xrange(0, len(result), 8)]

    def to_num(bits):
        n = 0
        for i in xrange(len(bits)):
            if bits[i]:
                n |= (1 << i)

        return n

    print 'Total bytes = {}'.format(len(result))

    result = map(to_num, result)

    print ', '.join(map(lambda x: '0x{:02X}'.format(x), result))


mppt = [
    (12, 0x111),
    (12, 0x222),
    (12, 0x333),
    (12, 0x444),
    (3, 0b111)
]

distr = [
    (10, 0x3AA),
    (10, 0x3FF),
    (10, 0x3BB),
    (10, 0x3EE),
    (10, 0x3CC),
    (10, 0x3DD),
    (7, 0b1111101),
    (6, 0b010111),
]

batc = [
    (10, 0x3FF),
    (10, 0x3FF),
    (10, 0x3FF),
    (10, 0x3FF),
    (3, 0b111)
]

bp = [
    (13, 0xABC),
    (13, 0xDEF)
]

current = [
    (8, 0xFF),
    (16, 0xffff),
    (32, 0xffffffff),
    (10, 0x3ff),
    (10, 0x3ff)
]

other = [
    (10, 987)
]

dcdc3v3 = [
    (10, 0x3ff)
]

dcdc5v = [
    (10, 0x3de)
]

encode(mppt * 3 + distr + batc + bp + current + other + dcdc3v3 + dcdc5v)
