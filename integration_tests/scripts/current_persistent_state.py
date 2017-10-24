from IPython.core.display import display

from persistent_state import PersistentStateParser
from response_frames.persistent_state import PersistentStateFrame
from telecommand import *
from utils import ensure_string

system.comm.put_frame(GetPersistentState())

ps1 = system.comm.get_frame(1000, filter_type=PersistentStateFrame)
ps2 = system.comm.get_frame(1000, filter_type=PersistentStateFrame)

raw_persistent_state = ps1.payload() + ps2.payload()

display(PersistentStateParser.parse(ensure_string(raw_persistent_state)))