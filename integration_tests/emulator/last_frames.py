from datetime import datetime
from wx import xrc, ListBox

from .base import ModuleBase


class LastFramesModule(ModuleBase):
    def __init__(self, last_frames):
        self._last_frames = last_frames
        self._current_items = []
        self.title = 'Last frames'
        self.grid_span = (1, 1)
        self.grid_pos = (1, 4)

    def load(self, res, parent):
        self._panel = res.LoadPanel(parent, 'LastFramesModule')

        self._list = xrc.XRCCTRL(self._panel, 'last_frames') # type: ListBox

    def root(self):
        return self._panel

    def update(self):
        for idx in xrange(len(self._current_items) - 1, -1, -1):
            to_remove = self._current_items[idx]

            if to_remove not in self._last_frames:
                self._list.Delete(idx)

        for idx in xrange(len(self._last_frames) - 1, -1, -1):
            to_add = self._last_frames[idx]

            if to_add not in self._current_items:
                text = '{:%Y-%m-%d %H:%M:%S}: {}'.format(datetime.now(), str(to_add))
                self._list.Insert(text, 0)

        self._current_items = list(self._last_frames)