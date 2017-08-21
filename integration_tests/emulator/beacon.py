from .base import ModuleBase
import wx
from wx import propgrid
from wx import xrc


class BeaconModule(ModuleBase):
    def __init__(self, last_beacon):
        self._last_beacon = last_beacon
        self.title = 'Beacon'
        self.grid_pos = (0, 0)
        self.grid_span = (2, 0)

    def load(self, res, parent):
        self._panel = res.LoadPanel(parent, 'BeaconModule')  # type: wx.Panel
        self._props = propgrid.PropertyGrid(parent=self._panel)
        self._panel.GetSizer().Add(self._props, 1, wx.EXPAND)

    def root(self):
        return self._panel

    def _update_node(self, node, tree):
        # type: (propgrid.PGProperty, any) -> None

        for key in tree:
            value = tree[key]

            existing_property = node.GetPropertyByName(key)  # type: propgrid.PGProperty

            if existing_property is None:
                self._props._AutoFillOne(node, key, value)
            elif type(value) is dict:
                self._update_node(existing_property, value)
            else:
                existing_property.SetValue(value)

    def update(self):
        self._update_node(self._props.GetGrid().GetRoot(), self._last_beacon)

        self._props.Refresh()
