import _dk_core as core
from .. import blendstate
from .view import View


class _Dimmer(View):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.alpha = 1.0
        self.delay = 1.0

    def onLoaded(self):
        super().onLoaded()
        self.setBlendState(blendstate.defaultAlpha)
        self.backgroundColor = core.Color(0, 0, 0, 0)
        self.screen().postOperation(self.setContentViewFrameToCenter, ())

    def onUnload(self):
        super().onUnload()

    def onUpdate(self, delta, tick, date):
        parent = self.parent()
        scale = parent.contentScale
        lin = core.LinearTransform2()
        lin.scale(scale)
        self.transform = core.AffineTransform2(lin).matrix3()

        a = self.backgroundColor.a
        if round(a * 255) < round(self.alpha * 255):
            if self.delay > 0:
                offset = self.alpha * delta / self.delay
                a += offset
            else:
                a = self.alpha

            if a > self.alpha:
                a = self.alpha

            self.backgroundColor.a = a
            self.redraw()

        if len(self.children()) == 0:
            self.screen().postOperation(self.removeFromParent, ())

    def setContentViewFrameToCenter(self):
        view = self.children()[0]
        frame = view.frame
        frame.center = self.bounds().center
        frame.origin = round(frame.x), round(frame.y)
        view.setFrame(frame)

    def onRender(self, renderer):
        renderer.clear(self.backgroundColor)

    def getResolution(self):
        parent = self.parent()
        if parent:
            return parent.contentResolution
        return self.screen().resolution

    def removeFromParent(self, *args):
        parent = self.parent()
        if parent:
            m = Modal.modalForView(parent)
            if m:
                screen = self.screen()
                if screen:
                    screen.postOperation(m.dismiss, (0,))
                    return
        return super().removeFromParent(*args)

_modalViewForFrame = {}

class Modal:

    fadeOutAlpha = 0.76
    fadeOutDelay = 0.25

    def __init__(self, view):

        self.callback = None
        self._view = _Dimmer()

        assert isinstance(view, View)
        self._view.addChild(view)

    def present(self, view, callback=None):
        if view in _modalViewForFrame:
            raise RuntimeError('modal for view is already exist!')

        assert isinstance(view, core.Frame)
        if callback:
            assert callable(callback)

        self._view.alpha = self.fadeOutAlpha
        self._view.delay = self.fadeOutDelay

        view.addChild(self._view)
        view.bringChildToFront(self._view)
        self.callback = callback
        _modalViewForFrame[view] = self

    def dismiss(self, result):
        frame = self._view.parent()
        if frame:
            del _modalViewForFrame[frame]
            View.removeFromParent(self._view, True)
            if self.callback:
                self.callback(self, result)

        self._view.unload()

    @staticmethod
    def modalForView(view):
        return _modalViewForFrame.get(view)
