import os
import sys

import imp

try:
    from obc import OBC, SerialPortTerminal
except ImportError:
    sys.path.append(os.path.join(os.path.dirname(__file__), '..'))
    from obc import OBC, SerialPortTerminal

import argparse
from IPython.terminal.embed import InteractiveShellEmbed
from IPython.terminal.prompts import Prompts
from pygments.token import Token
from traitlets.config.loader import Config
import socket
from utils import ensure_string
from devices import UplinkFrame

parser = argparse.ArgumentParser()

parser.add_argument('-c', '--config', required=True,
                    help="Config file (in CMake-generated integration tests format, only MOCK_COM required)")
parser.add_argument('-t', '--target', required=True,
                    help="Host with Just-Mocks", default='localhost')
parser.add_argument('-p', '--port', required=True,
                    help="Just-Mocks port", default=1234, type=int)

args = parser.parse_args()

imp.load_source('config', args.config)

def read_all(s, size):
    result = ""
    while size > 0:
        part = s.recv(size)
        result += part
        size -= len(part)

    return result


def send(frame):
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect(('127.0.0.1', 1234))

    f = frame.build()
    raw = 'S' + chr(len(f)) + ensure_string(f)

    sock.sendall(raw)

    data = read_all(sock, 3)

    sock.close()

    return data == 'ACK'


class MyPrompt(Prompts):
    def in_prompt_tokens(self, cli=None):
        return [(Token.Prompt, 'COMM'),
                (Token.Prompt, '> ')]


cfg = Config()
cfg.InteractiveShellEmbed.exec_lines.append('print "Dupa"')
shell = InteractiveShellEmbed(config=cfg, user_ns={'send': send, 'f': UplinkFrame(ord('P'), 'ABC')},
                              banner2='COMM Terminal')
shell.prompts = MyPrompt(shell)
shell.run_code('from telecommand import *')
shell()
