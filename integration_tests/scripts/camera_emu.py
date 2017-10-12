from i2cMock import MockPin

system.eps.CamWing.on_enable = lambda *args: system.i2c.gpio_low(MockPin.PC10)
system.eps.CamWing.on_disable = lambda *args: system.i2c.gpio_high(MockPin.PC10)

system.eps.CamNadir.on_enable = lambda *args: system.i2c.gpio_low(MockPin.PC12)
system.eps.CamNadir.on_disable = lambda *args: system.i2c.gpio_high(MockPin.PC12)


def download_photo(obc, local):
    data = system.obc.read_file(obc)

    with open(local, 'wb') as f:
        f.write(data)

    result = []

    rem = data[4:]

    while len(rem) > 0:
        part = rem[0:512 - 6]

        result += part

        rem = rem[512:]
        rem = rem[0:]

    with open(local + '.jpg', 'wb') as f:
        f.write(''.join(result))