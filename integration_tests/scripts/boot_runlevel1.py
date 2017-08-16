# Run with %run -i
from obc.boot import *


system.restart(boot_chain=[SelectRunlevel(1)])