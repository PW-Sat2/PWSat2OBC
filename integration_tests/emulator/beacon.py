from threading import Thread, Lock

import wx
import zmq
from struct import pack

from datetime import datetime
from wx import propgrid

from devices.comm_beacon import BeaconFrame
from .beacon_parser.full_beacon_parser import FullBeaconParser
from .beacon_parser.parser import BeaconStorage, BitArrayParser
from utils import ensure_byte_list
from .base import ModuleBase


class BeaconModule(ModuleBase):
    type_to_property = {
        int: propgrid.IntProperty,
        str: propgrid.StringProperty
    }

    def __init__(self, system, last_beacon):
        self._last_beacon = last_beacon
        self._system = system
        self.title = 'Beacon'
        self.grid_pos = (0, 0)
        self.grid_span = (2, 0)

    def load(self, res, parent):
        self._panel = res.LoadPanel(parent, 'BeaconModule')  # type: wx.Panel
        self._props = propgrid.PropertyGrid(parent=self._panel, style=propgrid.PG_TOOLTIPS | propgrid.PG_AUTO_SORT)
        self._props.SetExtraStyle(propgrid.PG_EX_HELP_AS_TOOLTIPS)
        self._panel.GetSizer().Add(self._props, 1, wx.EXPAND)

    def root(self):
        return self._panel

    def _update_node(self, node, tree):
        # type: (propgrid.PGProperty, any) -> None

        for key in tree:
            value = tree[key]

            if value is None:
                continue

            name = node.GetName() + '.' + key

            existing_property = node.GetPropertyByName(name)  # type: propgrid.PGProperty

            if existing_property is None:
                if type(value) is dict:
                    n = propgrid.PropertyCategory(label=key, name=name)
                    node.AppendChild(n)
                    self._update_node(n, value)
                    self._props.Collapse(n)
                else:
                    n = propgrid.StringProperty(label=key, name=name, value=str(value))
                    n.SetHelpString(key + ': ' + repr(value))

                    node.AppendChild(n)
                    self._props.SetPropertyReadOnly(n)
            else:
                if type(value) is dict:
                    self._update_node(existing_property, value)
                else:
                    existing_property.SetValue(str(value))
                    existing_property.SetHelpString(key + ': ' + repr(value))

    def update(self):
        if self._last_beacon.payload is not None:
            b = self._last_beacon.payload
            store = BeaconStorage()
            parser = BitArrayParser(FullBeaconParser(),
                                    ''.join(map(lambda x: pack('B', x), b.payload())),
                                    store)
            parser.parse()

            self._update_node(self._props.GetGrid().GetRoot(), store.storage)

            self._props.Refresh()
