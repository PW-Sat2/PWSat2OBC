from datetime import datetime

from nose.tools import nottest
from nose_parameterized import parameterized

from system import wait_for_obc_start
from tests.base import BaseTest


class FileSystemTests(BaseTest):
    @wait_for_obc_start()
    def test_write_read_file(self):
        text = datetime.now().isoformat()
        self.system.obc.write_file("/a/test_file", text)

        files = self.system.obc.list_files("/a")
        self.assertIn("test_file", files)

        self.system.obc.power_off()
        self.system.obc.power_on(clean_state=False)
        self.system.obc.wait_to_start()

        read_back = self.system.obc.read_file("/a/test_file")
        self.assertEqual(read_back, text)

    @parameterized.expand([
        ("/a",)
    ])
    @wait_for_obc_start()
    def test_write_read_long_file(self, base):
        path = base + "/file"
        data = '\n'.join(map(lambda x: x * 25, ['A', 'B', '>', 'C', 'D', 'E', 'F', 'G']))

        self.system.obc.write_file(path, data)

        files = self.system.obc.list_files(base)

        self.assertIn('file', files)

        read_back = self.system.obc.read_file(path)

        self.assertEqual(read_back, data)

