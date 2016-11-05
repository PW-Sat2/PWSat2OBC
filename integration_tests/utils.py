def hex_data(data):
    if isinstance(data, basestring):
        data = [ord(c) for c in data]

    return str(['%X' % b for b in data])


def pad_to_multiply(s, base):
    l = base - len(s) % base
    if l < base:
        padding = '=' * l
    else:
        padding = ''
    return s + padding


def b64pad(s):
    return pad_to_multiply(s, 3)