import telecommand
from datetime import timedelta
import time
import wx
from wx import xrc
from devices import RTCDevice
from .base import ModuleBase, bind
from bitarray import bitarray
from struct import pack

from beacon_parser.full_beacon_parser import FullBeaconParser

class CommModule(ModuleBase):
    GridPos = (1, 0)

    def __init__(self, system):
        self._system = system
        self.title = 'Comm'
        self.grid_pos = (1, 3)
        self.grid_span = (1, 1)
        self._last_beacon_seen = None
        self._last_mission_time_seen = 0

    def load(self, res, parent):
        self._panel = res.LoadPanel(parent, 'COMMModule')

        self.bind_handlers()

        self._current_comm_queue_size = xrc.XRCCTRL(self._panel, 'current_comm_queue_size')

        self._current_beacon_timestamp = xrc.XRCCTRL(self._panel, 'current_beacon_timestamp')

        self._beaconFrame = res.LoadFrame(None, 'BeaconFrame')  # type: wx.Frame
        self._beaconFrame.SetDoubleBuffered(True)
        self._beaconFrame.Bind(wx.EVT_CLOSE, self._onFrameClose)

        self._fileTree = xrc.XRCCTRL(self._beaconFrame, 'beacon_tree')  # type: wx.TreeCtrl
        self._root = self._fileTree.AddRoot('Beacon')

        self._fileTree.AppendItem(self._root, '0: No data laoded')

        self._fileTree.ExpandAllChildren(self._root)

    def root(self):
        return self._panel

    @bind('send_beacon', wx.EVT_BUTTON)
    def _send_beacon(self, evt):
        self._system.comm.put_frame(telecommand.SendBeacon())
        beacon_frame = self._system.comm.get_frame(30)
        self._system.transmitter.current_beacon_timestamp = time.localtime()
        self._system.transmitter.current_beacon = beacon_frame.payload()

    @bind('comm_reset_queue', wx.EVT_BUTTON)
    def _reset_queue(self, evt):
        self._system.transmitter.reset()

    @bind('comm_get_frame', wx.EVT_BUTTON)
    def _get_frame(self, evt):
        self._system.comm.get_frame(20)

    @bind('show_beacon_frame', wx.EVT_BUTTON)
    def _showFrame(self, evt):
        self._beaconFrame.Show()

    def _onFrameClose(self, evt):
        self._beaconFrame.Iconize(True)

    def update(self):
        self._current_comm_queue_size.SetLabel('Comm queue size: ' + str(self._system.transmitter.queue_size()))

        if time.mktime(time.localtime()) - self._last_mission_time_seen > 4:
            self._last_mission_time_seen = time.mktime(time.localtime())

        if self._system.transmitter.current_beacon_timestamp is not None:
            self._current_beacon_timestamp.SetLabel(
                'Current beacon: \n' + time.strftime('%Y-%m-%d %H:%M:%S', self._system.transmitter.current_beacon_timestamp)
                + ' (' + str(len(self._system.transmitter.current_beacon)) + ' bytes)')

        if self._last_beacon_seen != self._system.transmitter.current_beacon_timestamp:

            self._fileTree.DeleteChildren(self._root)
            parsers = FullBeaconParser().GetParsers(self._fileTree)

            # load the becon
            all_bits = bitarray(endian='little')
            all_bits.frombytes( ''.join(map(lambda x: pack('B', x), self._system.transmitter.current_beacon)))

            parsers.reverse()
            start_index = 0

            while len(parsers) > 0:
                parser = parsers.pop()
                to_take = parser.get_bit_count()
                end_index = to_take + start_index

                to_parse = all_bits[start_index:end_index]

                parser.parse(start_index, to_parse)
                start_index += to_take

            self._fileTree.ExpandAllChildren(self._root)
            self._last_beacon_seen = self._system.transmitter.current_beacon_timestamp


