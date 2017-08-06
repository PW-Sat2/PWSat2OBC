import argparse
import imp
import os
import sys

from IPython.terminal.prompts import Prompts
from pygments.token import Token
from traitlets.config.loader import Config
from IPython.terminal.embed import InteractiveShellEmbed


parser = argparse.ArgumentParser()

parser.add_argument('-c', '--config', required=True,
                    help="Config file (in CMake-generated integration tests format, only MOCK_COM required)", )

args = parser.parse_args()

config = imp.load_source('config', args.config)

try:
    from obc import OBC, SerialPortTerminal
except ImportError:
    sys.path.append(os.path.join(os.path.dirname(__file__), '..'))
    from obc import OBC, SerialPortTerminal


class DummyGPIO:
    RESET = 0

    def high(self, _):
        pass

    def low(self, _):
        pass

class MyPrompt(Prompts):
    def in_prompt_tokens(self, cli=None):
        return [(Token.Prompt, 'OBC'),
                (Token.Prompt, '> ')]

obc = OBC(SerialPortTerminal(config.config['OBC_COM'], DummyGPIO()))

cfg = Config()
shell = InteractiveShellEmbed(config=cfg, user_ns={'obc': obc}, banner2='OBC Terminal')
shell.prompts = MyPrompt(shell)
shell()
