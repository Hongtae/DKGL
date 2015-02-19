import _dk_core as core
from .. import frame
from .. import blendstate
from . import resource
from . import font


DEFAULT_UI_SCALE = 1        # use 2 for retina-display


class View(frame.Frame):

    fontAttributes = font.attributes(12)

    backgroundColor = core.Color(1.0, 1.0, 1.0)

    borderColor = core.Color(0.0, 0.0, 0.0, 1.0)
    borderWidth = 0

    minimumViewWidth = 1
    minimumViewHeight = 1

    def __init__(self, frame=core.Rect(0, 0, 1, 1), *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.scaleFactor = DEFAULT_UI_SCALE
        assert self.scaleFactor > 0
        assert isinstance(frame, core.Rect)

        self.layouter = None
        self.gestureRecognizers = []

        self.setFrame(frame)
        self.setBlendState(blendstate.defaultOpaque)
        self.font = None

    def layout(self):
        if self.layouter:
            self.layouter(self.contentBounds(), self.children())

    def onResized(self):
        self.contentScale = tuple(v / self.scaleFactor for v in self.contentResolution)
        self.surfaceVisibilityTest = not (round(self.borderWidth) > 0)
        self.layout()

    @property
    def frame(self):
        return core.Rect(self.__frame)

    @frame.setter
    def frame(self, frame):
        self.setFrame(frame)

    def setFrame(self, frame):
        assert isinstance(frame, core.Rect)
        #assert frame.width > 0
        #assert frame.height > 0
        assert self.minimumViewWidth > 0
        assert self.minimumViewHeight > 0
        width = max(frame.width, self.minimumViewWidth)
        height = max(frame.height, self.minimumViewHeight)

        frame = core.Rect(frame.origin + (width, height))

        try:
            f = self.__frame
            if f == frame:
                return
        except AttributeError:
            pass

        self.__frame = frame
        self.contentScale = width, height

        linear = core.LinearTransform2()
        linear.scale(width, height)
        self.transform = core.AffineTransform2(linear, frame.origin).matrix3()

    def bounds(self):
        '''
        :return: projected local bounds rect (include border)
        '''
        w, h = self.contentScale
        return core.Rect(0, 0, w, h)

    def contentBounds(self):
        '''
        :return: projected local bounds rect for contents (without border)
        '''
        rc = self.bounds()
        border = round(self.borderWidth)
        border2 = border * 2
        rc.size = (rc.width - border2, rc.height - border2)
        rc.origin = (rc.x + border, rc.y + border)
        return rc

    def displayBounds(self):
        '''
        :return: unprojected local bounds rect (include border)
        '''
        return self.unprojectLocalRect(self.bounds())

    def contentDisplayBounds(self):
        '''
        :return: unprojected local bounds rect for contents (without border)
        '''
        return self.unprojectLocalRect(self.contentBounds())

    def onRender(self, renderer):
        border = round(self.borderWidth)
        if border > 0:
            renderer.clear(self.borderColor)

            bounds = View.contentBounds(self)
            pixelBounds = self.convertLocalToPixel(self.unprojectLocalRect(bounds))

            renderer.viewport = pixelBounds.tuple
            renderer.bounds = bounds.tuple

            _tm = renderer.transform
            renderer.transform = core.Matrix3()

            with renderer.contextForSolidRects(self.backgroundColor, blend=blendstate.defaultOpaque) as r:
                r.add(bounds)

            renderer.transform = _tm

        else:
            renderer.clear(self.backgroundColor)

    def onUpdate(self, delta, tick, date):
        pass

    def onLoaded(self):
        super().onLoaded()
        self.contentScale = tuple(v / self.scaleFactor for v in self.contentResolution)
        border2 = round(self.borderWidth) * 2
        minWidth = border2 + 1
        minHeight = border2 + 1
        self.minimumViewWidth = max(minWidth, self.minimumViewWidth)
        self.maximumViewHeight = max(minHeight, self.minimumViewHeight)
        if not self.font:
            self.font = font.loadUIFont(self.fontAttributes, self.scaleFactor)
        frame = self.frame
        if frame.width < self.minimumViewWidth:
            frame.height = self.minimumViewWidth
        if frame.height < self.minimumViewHeight:
            frame.height = self.minimumViewHeight
        self.frame = frame

    def onUnload(self):
        self.font = None
        screen = self.screen()
        if screen and screen.frame == self:
            resource.clear()
        super().onUnload()

    def getResolution(self):
        return tuple(v * self.scaleFactor for v in self.__frame.size)

    def contentHitTest(self, pos):
        return self.contentDisplayBounds().isInside(pos)

    def captureMouse(self, deviceId):
        r = super().captureMouse(deviceId)
        screen = self.screen()
        if screen:
            keyboardId = 0
            kf = screen.keyFrame(keyboardId)
            if kf and not kf.isDescendantOf(self):
                self.captureKeyboard(keyboardId)
                self.releaseKeyboard(keyboardId)
        return r
