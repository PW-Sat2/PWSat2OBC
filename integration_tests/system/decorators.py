from functools import wraps

from obc.boot import SelectRunlevel, ClearState


def auto_comm_handling(enable_auto_comm):
    def wrap(f):
        @wraps(f)
        def wrapper(self, *args, **kwargs):
            self.system.obc.wait_to_start()
            self.system.obc.comm_auto_handling(enable_auto_comm)
            return f(self, *args, **kwargs)

        return wrapper

    return wrap


def auto_power_on(auto_power_on):
    def wrap(f):
        @wraps(f)
        def wrapper(self, *args, **kwargs):
            result = f(self, *args, **kwargs)
            self.auto_power_on = auto_power_on
            return result
        return wrapper
    return wrap


def runlevel(requested_runlevel):
    def wrap(f):
        try:
            f.boot_wrappers += [SelectRunlevel(requested_runlevel)]
        except AttributeError:
            f.boot_wrappers = [SelectRunlevel(requested_runlevel)]
        return f

    return wrap


def clear_state():
    def wrap(f):
        try:
            f.boot_wrappers += [ClearState()]
        except AttributeError:
            f.boot_wrappers = [ClearState()]
        return f

    return wrap
