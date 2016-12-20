from datetime import datetime
from system import wait_for_obc_start
from tests.base import BaseTest


class FileSystemTests(BaseTest):
    @wait_for_obc_start()
    def test_write_read_file(self):
        text = datetime.now().isoformat()
        self.system.obc.write_file("/test_file", text)

        files = self.system.obc.list_files("/")
        self.assertIn("test_file", files)

        self.system.obc.power_off()
        self.system.obc.power_on(clean_state=False)
        self.system.obc.wait_to_start()

        read_back = self.system.obc.read_file("/test_file")
        self.assertEqual(read_back, text)
