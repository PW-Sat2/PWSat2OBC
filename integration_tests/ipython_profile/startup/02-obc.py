from obc.boot import BootToUpper, BootToIndex


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
    mock_com = config['MOCK_COM']
    gpio_com = config['GPIO_COM']
    boot_handler = BootToUpper() if config['BOOT_UPPER'] else BootToIndex(config['BOOT_INDEX'])

    gpio = Pins(gpio_com)

    return System(obc_com, mock_com, gpio, boot_handler)

system = _prepare_environment()