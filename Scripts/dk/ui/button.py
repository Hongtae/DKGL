import _dk_core as core
from .. import renderer
from .. import blendstate
from . import view
from . import imageview
from . import label
from . import control
from . import resource
from . import font


class Button(label.Label, control.Control, imageview.ImageView):

    fontAttributes = font.attributes(12)

    backgroundColor = core.Color(0.85, 0.85, 0.85)
    backgroundColorHighlighted = core.Color(0.9, 0.9, 1.0)
    backgroundColorActivated = core.Color(0.5, 0.5, 0.5)
    backgroundColorDisabled = core.Color(0.4, 0.4, 0.4)

    textColor = core.Color(0.0, 0.0, 0.0)
    textColorHighlighted = core.Color(0.0, 0.0, 0.0)
    textColorActivated = core.Color(1.0, 1.0, 1.0)
    textColorDisabled = core.Color(0.3, 0.3, 0.3)

    outlineColor = None
    outlineColorHighlighted = core.Color(1.0, 1.0, 1.0, 1.0)
    outlineColorActivated = core.Color(0.0, 0.0, 0.0, 1.0)
    outlineColorDisabled = None

    backgroundImage = None
    backgroundImageHighlighted = None
    backgroundImageActivated = None
    backgroundImageDisabled = None

    backgroundImageTransform = core.Matrix3()

    borderColor = core.Color(0.0, 0.0, 0.0, 1.0)
    borderWidth = 1


    def __init__(self, text='Button', *args, **kwargs):
        super().__init__(text=text, *args, **kwargs)

        self.buttonPressed = False
        self.__mouseHover = False
        self.__capturedMouseId = None
        if self.textureImage:
            self.backgroundImage = self.textureImage

    def setTextColor(self, color, state=control.Control.STATE_ALL):
        assert isinstance(color, core.Color)
        if state in (self.STATE_ALL, self.STATE_NORMAL):
            self.textColor = color
        if state in (self.STATE_ALL, self.STATE_HIGHLIGHTED):
            self.textColorHighlighted = color
        if state in (self.STATE_ALL, self.STATE_ACTIVATED):
            self.textColorActivated = color
        if state in (self.STATE_ALL, self.STATE_DISABLED):
            self.textColorDisabled = color

    def setOutlineColor(self, color, state=control.Control.STATE_ALL):
        assert isinstance(color, core.Color)
        if state in (self.STATE_ALL, self.STATE_NORMAL):
          self.outlineColor = color
        if state in (self.STATE_ALL, self.STATE_HIGHLIGHTED):
          self.outlineColorHighlighted = color
        if state in (self.STATE_ALL, self.STATE_ACTIVATED):
          self.outlineColorActivated = color
        if state in (self.STATE_ALL, self.STATE_DISABLED):
            self.outlineColorDisabled = color


    def onLoaded(self):
        super().onLoaded()


    def onUnload(self):
        self.backgroundImage = None
        self.backgroundImageHighlighted = None
        self.backgroundImageActivated = None
        self.backgroundImageDisabled = None
        return super().onUnload()

    def onRender(self, r):

        state = self.STATE_DISABLED if not self.enabled else \
                self.STATE_ACTIVATED if self.buttonPressed else \
                self.STATE_HIGHLIGHTED if self.__mouseHover else \
                self.STATE_NORMAL

        textColors = (self.textColor,
                      self.textColorHighlighted,
                      self.textColorActivated,
                      self.textColorDisabled)

        outlineColors = (self.outlineColor,
                         self.outlineColorHighlighted,
                         self.outlineColorActivated,
                         self.outlineColorDisabled)

        bgColors = (self.backgroundColor,
                    self.backgroundColorHighlighted,
                    self.backgroundColorActivated,
                    self.backgroundColorDisabled)

        bgImages = (self.backgroundImage,
                    self.backgroundImageHighlighted,
                    self.backgroundImageActivated,
                    self.backgroundImageDisabled)

        self.textColor = textColors[state]
        self.outlineColor = outlineColors[state]
        self.backgroundColor = bgColors[state]
        self.textureImage = bgImages[state]
        if not self.textureImage:
            self.textureImage = self.backgroundImage

        super().onRender(r)
        self.backgroundColor = bgColors[0]
        self.textColor = textColors[0]
        self.outlineColor = outlineColors[0]


    def discardAction(self):
        if self.__capturedMouseId:
            if self.isMouseCapturedBySelf(self.__capturedMouseId[0]):
                self.releaseMouse(self.__capturedMouseId[0])
            self.__capturedMouseId = None
            self.buttonPressed = False
            self.redraw()

    def onMouseDown(self, deviceId, buttonId, pos):
        super().onMouseDown(deviceId, buttonId, pos)
        acceptMouse = True
        if self.__capturedMouseId:
            if not self.isMouseCapturedBySelf(self.__capturedMouseId[0]):
                self.__capturedMouseId = None

        if self.__capturedMouseId is None:
            self.captureMouse(deviceId)
            self.__capturedMouseId = (deviceId, buttonId)
            self.buttonPressed = True
            self.redraw()

    def onMouseUp(self, deviceId, buttonId, pos):
        super().onMouseUp(deviceId, buttonId, pos)
        if self.__capturedMouseId and self.__capturedMouseId == (deviceId, buttonId):
            self.releaseMouse(deviceId)
            if self.bounds().isInside(pos):
                self.screen().postOperation(self.postEvent, ())

            self.__capturedMouseId = None
            self.buttonPressed = False
            self.redraw()

    def onMouseMove(self, deviceId, pos, delta):
        super().onMouseMove(deviceId, pos, delta)
        if self.__capturedMouseId and self.__capturedMouseId[0] == deviceId:
            btnPressed = self.buttonPressed
            if self.isMouseCapturedBySelf(deviceId):
                self.buttonPressed = self.bounds().isInside(pos)
            else:
                self.__capturedMouseId = None
                self.buttonPressed = False

            if btnPressed != self.buttonPressed:
                self.redraw()

        elif deviceId == 0:
            if not self.__mouseHover:
                self.__mouseHover = True
                if not self.buttonPressed:
                    self.redraw()

    def onMouseLeave(self, deviceId):
        super().onMouseLeave(deviceId)
        if deviceId == 0 and self.__mouseHover:
            self.__mouseHover = False
            if not self.buttonPressed:
                self.redraw()

    def postEvent(self):
        super().invokeAllTargets(self)