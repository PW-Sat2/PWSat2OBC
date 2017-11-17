def on_power_cycle(*args):
    def p():
        print 'Power cycling!'
        from obc.boot import NormalBoot
        system.restart(override_final=NormalBoot())
        print 'power cycle done!'

    from threading import Timer

    Timer(0.1, p).start()

system.eps.controller_a.on_power_cycle = on_power_cycle
system.eps.controller_b.on_power_cycle = on_power_cycle
