from datetime import datetime

from tests.base import BaseTest


class FileSystemTests(BaseTest):
    def test_write_read_file(self):
        text = datetime.now().isoformat()
        self.system.obc.write_file("/test_file", text)
        files = self.system.obc.list_files("/")

        self.assertIn("test_file", files)

        read_back = self.system.obc.read_file("/test_file")

        self.assertEqual(read_back, text)
