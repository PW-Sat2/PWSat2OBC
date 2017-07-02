import win32console


class Win32ReportProgress:
    def __init__(self):
        self._title = None
        self._test_counter = 0

    def set_up_once(self):
        self._title = win32console.GetConsoleTitle()

    def set_up(self, test_id, **kwargs):
        self._test_counter += 1

        title = '[{}] {}'.format(self._test_counter, test_id)
        win32console.SetConsoleTitle(title)

    def tear_down(self, test_id, **kwargs):
        pass

    def tear_down_once(self):
        if self._title is not None:
            win32console.SetConsoleTitle(self._title)
