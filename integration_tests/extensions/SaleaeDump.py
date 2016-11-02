import os
from time import sleep
from os import path

from saleae import Saleae


class SaleaeDump:
    _connection = Saleae

    def __init__(self, captures_folder):
        self._captures_folder = captures_folder

    def set_up_once(self):
        self._connection = Saleae()
        self._connection.set_capture_seconds(90)

    def set_up(self, **kwargs):
        self._connection.capture_start()
        sleep(0.5)

    def tear_down(self, test_id, **kwargs):
        sleep(1)
        self._connection.capture_stop()

        if not path.exists(self._captures_folder):
            os.makedirs(self._captures_folder)

        output_file = path.join(self._captures_folder, '%s.logicdata' % test_id)

        self._connection.save_to_file(output_file)

    def tear_down_once(self):
        pass