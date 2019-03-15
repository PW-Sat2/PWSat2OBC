def on_power_cycle(*args):
    def p():
        print 'Power cycling!'
        from obc.boot import NormalBoot
        from time import sleep
        system.i2c.unlatch()
        system.obc._terminal._gpio.low(system.obc._terminal._gpio.RESET)
        sleep(0.1)
        system.obc._terminal._gpio.high(system.obc._terminal._gpio.RESET)
        print 'power cycle done!'

    from threading import Timer

    Timer(0.1, p).start()


system.eps.controller_a.on_power_cycle = on_power_cycle
system.eps.controller_b.on_power_cycle = on_power_cycle
