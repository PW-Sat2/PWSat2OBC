from datetime import timedelta
import time
import wx
from wx import xrc
from devices import RTCDevice
from .base import ModuleBase, bind
from utils import bytes_to_dword, bytes_to_word, bytes_to_qword, bits_to_dword, bits_to_byte, bits_to_word, bits_to_qword
from bitarray import bitarray
from struct import pack


class Parser:
    def __init__(self, tree_control, name):
        self._offset = 0
        self._name = name
        self._tree_control = tree_control
        self._root = tree_control.AppendItem(tree_control.GetRootItem(), name)

    def append_dword(self, address, bits, name, length=32):
        self._tree_control.AppendItem(self._root, str(address + self._offset) + ': '
                               + name + ' = '
                               + str(bits_to_dword(bits[self._offset:self._offset + length])))

        self._offset += length

    def append_byte(self, address, bits, name, length=8):
        self._tree_control.AppendItem(self._root, str(address + self._offset) + ': '
                               + name + ' = '
                               + str(bits_to_byte(bits[self._offset:self._offset + length])))

        self._offset += length

    def append_word(self, address, bits, name, length=16):
        self._tree_control.AppendItem(self._root, str(address + self._offset) + ': '
                               + name + ' = '
                               + str(bits_to_word(bits[self._offset:self._offset + length])))

        self._offset += length

    def append_qword(self, address, bits, name, length=64):
        self._tree_control.AppendItem(self._root, str(address + self._offset) + ': '
                               + name + ' = '
                               + str(bits_to_qword(bits[self._offset:self._offset + length])))

        self._offset += length
