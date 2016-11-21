import logging
import os
import colorlog
import extensions


if os.getenv("CLICOLOR_FORCE") == "1":
    print "Forcing colors"
    import colorama
    colorama.deinit()


def _setup_log():
    root_logger = logging.getLogger()

    handler = colorlog.StreamHandler()

    formatter = colorlog.ColoredFormatter(
        "%(log_color)s%(asctime)-15s %(levelname)s: [%(name)s] %(message)s",
        log_colors={
            'DEBUG':    'cyan',
            'INFO':     'green',
            'WARNING':  'yellow',
            'ERROR':    'red',
            'CRITICAL': 'red,bg_white',
        }
    )

    handler.setFormatter(formatter)

    root_logger.addHandler(handler)
    root_logger.setLevel(logging.DEBUG)


def setup():
    _setup_log()

    extensions.initialize_extensions()

    extensions.set_up_once()


def tearDown():
    extensions.tear_down_once()
