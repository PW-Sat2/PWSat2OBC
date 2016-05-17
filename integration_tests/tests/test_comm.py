import os

from tests.base import BaseTest


mock_com = os.environ.get('MOCK_COM')
obc_com = os.environ.get('OBC_COM')


class Test_Comm(BaseTest):
    def test_should_initialize_properly(self):
        self.assertTrue(self.transmitter.wait_for_reset(3))

    def test_should_send_frame(self):
        self.transmitter.wait_for_reset(3)

        self.obc.send_frame("ABC")
        msg = self.transmitter.get_message_from_buffer(3)

        self.assertEqual(msg, (65, 66, 67))

