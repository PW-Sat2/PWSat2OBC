import inspect
from wx import xrc


def bind(control_id, event, args=()):
    def wrap(f):
        if not hasattr(f, 'bind_to'):
            f.bind_to = []

        f.bind_to += [(control_id, event, args)]

        return f

    return wrap


class ModuleBase:
    def bind_handlers(self):
        handlers = map(lambda x: x[1], inspect.getmembers(self))
        handlers = filter(lambda x: inspect.ismethod(x), handlers)
        handlers = filter(lambda x: hasattr(x, 'bind_to'), handlers)

        for handler in handlers:
            for target in handler.bind_to:
                (control_id, event, args) = target
                l = (lambda h, a: lambda e: h(e, *a))(handler, args)
                self.root().Bind(event, l, id=xrc.XRCID(control_id))