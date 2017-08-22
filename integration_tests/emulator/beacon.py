from bitarray import bitarray
from struct import pack

from .beacon_parser.full_beacon_parser import FullBeaconParser
from .beacon_parser.parser import BitReader, BeaconStorage
from .base import ModuleBase
import wx
from wx import propgrid
from wx import xrc


class BeaconModule(ModuleBase):
    type_to_property = {
        int: propgrid.IntProperty
    }

    def __init__(self, last_beacon, system):
        self._last_beacon = last_beacon
        self._system = system
        self.title = 'Beacon'
        self.grid_pos = (0, 0)
        self.grid_span = (2, 0)

    def load(self, res, parent):
        self._panel = res.LoadPanel(parent, 'BeaconModule')  # type: wx.Panel
        self._props = propgrid.PropertyGrid(parent=self._panel,style=propgrid.PG_TOOLTIPS | propgrid.PG_AUTO_SORT)
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
                    factory = BeaconModule.type_to_property[type(value)]
                    n = factory(label=key, name=name, value=value)
                    node.AppendChild(n)
                    self._props.SetPropertyReadOnly(n)
            else:
                if type(value) is dict:
                    self._update_node(existing_property, value)
                else:
                    existing_property.SetValue(value)

    def update(self):
        if self._system.transmitter.current_beacon_timestamp is not None:
            all_bits = bitarray(endian='little')
            all_bits.frombytes(''.join(map(lambda x: pack('B', x), self._system.transmitter.current_beacon)))

            reader = BitReader(all_bits)
            store = BeaconStorage()

            parsers = FullBeaconParser().GetParsers(reader, store)
            parsers.reverse()

            while len(parsers) > 0:
                parser = parsers.pop()
                parser.parse()

            self._update_node(self._props.GetGrid().GetRoot(), store.storage)

            self._props.Refresh()
