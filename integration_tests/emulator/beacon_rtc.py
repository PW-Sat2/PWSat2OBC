from datetime import timedelta
import time
import wx
from wx import xrc
from devices import RTCDevice
from .base import ModuleBase, bind
from bitarray import bitarray
from struct import pack
from beacon_parser.error_counting_telementry import  ErrorCountingTelementry
from beacon_parser.program_state import ProgramStateParser
from beacon_parser.startup_parser import StartupParser
from beacon_parser.time_state import TimeState

class BeaconRTCModule(ModuleBase):
    GridPos = (1, 0)

    def __init__(self, system):
        self._system = system
        self._rtc = system.rtc  # type: RTCDevice
        self.title = 'RTC'
        self.grid_pos = (1, 0)
        self._last_beacon_seen = None

    def load(self, res, parent):
        self._panel = res.LoadPanel(parent, 'RTCModule')

        self.bind_handlers()

        self._time = xrc.XRCCTRL(self._panel, 'rtc_time')
        self._current_beacon_timestamp = xrc.XRCCTRL(self._panel, 'current_beacon_timestamp')

        self._beaconFrame = res.LoadFrame(None, 'BeaconFrame')  # type: wx.Frame
        self._beaconFrame.SetDoubleBuffered(True)

        # self._beaconFrame.Show()
        # self._beaconFrame.Icon = wx.Icon(os.path.join(os.path.dirname(__file__), 'icon.ico'), wx.BITMAP_TYPE_ICO)

        self._fileTree = xrc.XRCCTRL(self._beaconFrame, 'beacon_tree')  # type: wx.TreeCtrl
        self._root = self._fileTree.AddRoot('Beacon')

        self._fileTree.AppendItem(self._root, '0: No data laoded')

        self._fileTree.ExpandAllChildren(self._root)

    def root(self):
        return self._panel

    @bind('show_beacon_frame', wx.EVT_BUTTON)
    def _showFrame(self, evt):
        self._beaconFrame.Show()

    def update(self):
        t = self._rtc.response_time()
        self._time.SetLabel(t.strftime('%Y-%m-%d %H:%M:%S'))

        if self._system.transmitter.current_beacon_timestamp is not None:
            self._current_beacon_timestamp.SetLabel(
                'Current beacon: ' + time.strftime('%Y-%m-%d %H:%M:%S', self._system.transmitter.current_beacon_timestamp)
                + ' (' + str(len(self._system.transmitter.current_beacon)) + ' bytes)')

        if self._last_beacon_seen != self._system.transmitter.current_beacon_timestamp:

            self._fileTree.DeleteChildren(self._root)
            parsers = [StartupParser(self._fileTree),
                       ProgramStateParser(self._fileTree),
                       TimeState(self._fileTree),
                       ErrorCountingTelementry(self._fileTree)]

            # load the becon
            all_bits = bitarray()
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

    @bind('rtc_start', wx.EVT_BUTTON)
    def _on_start(self, evt):
        self._rtc.start_running()

    @bind('rtc_stop', wx.EVT_BUTTON)
    def _on_stop(self, evt):
        self._rtc.stop_running()

    @bind('rtc_advance_5min', wx.EVT_BUTTON, args=(timedelta(minutes=5),))
    def _on_advance(self, evt, interval):
        self._rtc.advance_by(interval)


