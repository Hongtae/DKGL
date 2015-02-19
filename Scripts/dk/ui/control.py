import _dk_core as core
from weakref import WeakKeyDictionary
from . import view


class Control(view.View):

    STATE_NORMAL = 0
    STATE_HIGHLIGHTED = 1
    STATE_ACTIVATED = 2
    STATE_DISABLED = 3

    STATE_ALL = 0xff

    removeAllTargetsWhenUnload = True

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.__targets = WeakKeyDictionary()

    def addTarget(self, key, callback):
        if callable(callback):
            self.__targets[key] = callback
        else:
            raise TypeError('func is not callable')
        pass

    def removeTarget(self, key):
        try:
            del self.__targets[key]
        except KeyError:
            pass

    def removeAllTargets(self):
        self.__targets = WeakKeyDictionary()

    def invokeAllTargets(self, *args):
        for cb in self.__targets.values():
            cb(*args)

    def invokeOneTarget(self, key, *args):
        cb = self.__targets[key]
        cb(*args)

    def onUnload(self):
        if self.removeAllTargetsWhenUnload:
            self.removeAllTargets()
        return super().onUnload()

