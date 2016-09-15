def hex_data(data):
    if isinstance(data, basestring):
        data = [ord(c) for c in data]

    return str(['%X' % b for b in data])