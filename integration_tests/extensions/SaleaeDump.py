from time import sleep

from saleae import Saleae


class SaleaeDump:
    _connection = Saleae

    def set_up_once(self):
        self._connection = Saleae()
        self._connection.set_capture_seconds(90)

    def set_up(self, **kwargs):
        self._connection.capture_start()
        sleep(0.5)

    def tear_down(self, test_id, **kwargs):
        sleep(1)
        self._connection.capture_stop()
        self._connection.save_to_file('D:/captures/%s.logicdata' % test_id)

    def tear_down_once(self):
        pass