def _prepare_environment():
    import logging

    from pins import Pins
    from system import System

    try:
        from build_config import config
    except ImportError as e:
        raise ImportError(
            "Error loading config: %s. Did you forget to add <build>/integration_tests to PYTHONPATH?" % e.message)

    logging.basicConfig(level=logging.DEBUG)

    obc_com = config['OBC_COM']
    sys_bus_com = config['SYS_BUS_COM']
    payload_bus_com = config['PAYLOAD_BUS_COM']
    use_single_bus = config['SINGLE_BUS']
    gpio_com = config['GPIO_COM']

    gpio = Pins(gpio_com)

    return System(obc_com, sys_bus_com, payload_bus_com, use_single_bus, gpio)

system = _prepare_environment()