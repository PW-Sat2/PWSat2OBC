import sys

from SaleaeDump import SaleaeDump
import os

_extensions = []


def initialize_extensions():
    from build_config import config

    if 'CAPTURE_LINES' in os.environ and os.environ['CAPTURE_LINES'] == '1':
        _extensions.append(SaleaeDump(config['CAPTURES_OUTPUT']))

    if sys.platform == 'win32':
        import report_progress
        _extensions.append(report_progress.Win32ReportProgress())


def set_up_once():
    for e in _extensions:
        e.set_up_once()


def set_up(**kwargs):
    for e in _extensions:
        e.set_up(**kwargs)


def tear_down(**kwargs):
    for e in _extensions:
        e.tear_down(**kwargs)


def tear_down_once():
    for e in _extensions:
        e.tear_down_once()
