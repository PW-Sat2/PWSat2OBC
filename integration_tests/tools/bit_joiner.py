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


true = 1
false = 0

deployment_switch_a = map(lambda x: (1, x), [
    false, true, false, true
])

deployment_switch_b = map(lambda x: (1, x), [
    true, false, true, false
])

last_stop_a = map(lambda x: (1, x), [
    true, false, true, false
])

last_stop_b = map(lambda x: (1, x), [
    false, true, false, true
])

burn_active_a = map(lambda x: (1, x), [
    false, true, true, false
])

burn_active_b = map(lambda x: (1, x), [
    true, false, false, true
])

indep_burn = [
    (1, 1),
    (1, 1)
]

ignore_switch = [
    (1, 1),
    (1, 0)
]

armed = [
    (1, 1),
    (1, 0)
]

counts_a = map(lambda x: (3, x), [
    1, 2, 3, 4
])

counts_b = map(lambda x: (3, x), [
    5, 6, 7, 7
])

times_a = map(lambda x: (8, x), [
    5, 10, 15, 20
])

times_b = map(lambda x: (8, x), [
    25, 30, 35, 40
])

encode(deployment_switch_a + deployment_switch_b + last_stop_a + last_stop_b + burn_active_a + burn_active_b
       + indep_burn + ignore_switch + armed
       + counts_a + counts_b
       + times_a + times_b
       )
