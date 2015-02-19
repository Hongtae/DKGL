import _dk_core as core
from . import control
from . import view
from .. import blendstate

class Slider(control.Control, view.View):

    thumbRadius = 15
    trackSize = 12
    borderWidth = 1

    borderColor = core.Color(0.0, 0.0, 0.0, 1.0)
    vertical = False

    trackColor = core.Color(0.8, 0.8, 0.5)
    trackColorHighlighted = core.Color(1.0, 1.0, 0.5)
    trackColorActivated = core.Color(1.0, 1.0, 0.68)
    trackColorDisabled = core.Color(0.1, 0.1, 0.1)

    thumbColor = core.Color(0.5, 0.5, 1.0)
    thumbColorHighlighted = core.Color(0.7, 0.7, 1.0)
    thumbColorActivated = core.Color(0.4, 0.4, 1.0)
    thumbColorDisabled = core.Color(0.1, 0.1, 0.5)

    thumbCoreColor = core.Color(0.0, 0.0, 1.0)
    thumbCoreColorHighlighted = core.Color(0.3, 0.3, 1.0)
    thumbCoreColorActivated = core.Color(0.8, 0.8, 1.0)
    thumbCoreColorDisabled = core.Color(0.4, 0.4, 0.4)

    minimumViewHeight = thumbRadius * 2
    minimumViewWidth = thumbRadius * 2

    def __init__(self, value=0.0, range=(0.0, 1.0), *args, **kwargs):
        super().__init__(*args, **kwargs)

        self.value = value
        self.minimumValue = range[0]
        self.maximumValue = range[1]
        self.__updatedValue = self.value

        self.__thumbActive = False
        self.__thumbHover = False
        self.__trackHover = False
        self.__capturedMouse = None

    def onLoaded(self):
        super().onLoaded()
        self.setBlendState(blendstate.defaultAlpha)

    def isActivated(self):
        return self.__thumbActive

    def thumbCenter(self):
        bounds = self.bounds()
        thumbPos = core.Point(bounds.center)

        range = self.maximumValue - self.minimumValue
        pos = (self.value - self.minimumValue) / range
        if self.vertical:
            trackingLength = bounds.height - self.thumbRadius * 2
            thumbPos.y = round(trackingLength * pos + self.thumbRadius)
        else:
            trackingLength = bounds.width - self.thumbRadius * 2
            thumbPos.x = round(trackingLength * pos + self.thumbRadius)

        return thumbPos

    def thumbRect(self):
        center = self.thumbCenter()
        rect = core.Rect(0, 0, self.thumbRadius*2, self.thumbRadius*2)
        rect.center = center
        return rect

    def trackRect(self):
        bounds = self.bounds()
        trackRect = core.Rect()
        if self.vertical:
            trackRect.origin = (self.thumbRadius - self.trackSize * 0.5, self.thumbRadius)
            trackRect.size = (self.trackSize, bounds.height - self.thumbRadius * 2)
        else:
            trackRect.origin = (self.thumbRadius, self.thumbRadius - self.trackSize * 0.5)
            trackRect.size = (bounds.width - self.thumbRadius * 2, self.trackSize)

        trackRect.center = bounds.center
        return trackRect

    def updateMouseHover(self, pos):
        thumbCenter = self.thumbCenter()
        length = (pos.vector() - thumbCenter.vector()).length()
        if length <= self.thumbRadius:
            if not self.__thumbHover or self.__trackHover:
                self.__thumbHover = True
                self.__trackHover = False
                self.redraw()
            return
        else:
            trackRect = self.trackRect()
            if trackRect.isInside(pos):
                if self.__thumbHover or not self.__trackHover:
                    self.__thumbHover = False
                    self.__trackHover = True
                    self.redraw()
                return

        if self.__thumbHover or self.__trackHover:
            self.__thumbHover = False
            self.__trackHover = False
            self.redraw()

    def onUpdate(self, delta, tick, date):
        assert self.maximumValue > self.minimumValue

        if self.value != self.__updatedValue:
            self.value = self.__updatedValue
            self.screen().postOperation(self.invokeAllTargets, (self,))
            self.redraw()


    def onRender(self, renderer):
        renderer.clear(core.Color(0,0,0,0))

        trackRect = self.trackRect()
        thumbRect = self.thumbRect()

        if self.borderWidth > 0:
            with renderer.contextForSolidRects(self.borderColor, blend=blendstate.defaultOpaque) as r2:
                r2.add(trackRect)

            trackRect.x += self.borderWidth
            trackRect.y += self.borderWidth
            trackRect.width -= self.borderWidth * 2
            trackRect.height -= self.borderWidth * 2

        trackState = self.STATE_NORMAL
        thumbState = self.STATE_NORMAL
        thumbCoreState = self.STATE_NORMAL

        if self.enabled:
            if self.__thumbActive:
                trackState = self.STATE_ACTIVATED
                thumbState = self.STATE_ACTIVATED
                thumbCoreState = self.STATE_ACTIVATED
            elif self.__trackHover:
                trackState = self.STATE_HIGHLIGHTED
                thumbState = self.STATE_NORMAL
                thumbCoreState = self.STATE_HIGHLIGHTED
            elif self.__thumbHover:
                trackState = self.STATE_NORMAL
                thumbState = self.STATE_HIGHLIGHTED
                thumbCoreState = self.STATE_HIGHLIGHTED
        else:
            trackState = self.STATE_DISABLED
            thumbState = self.STATE_DISABLED
            thumbCoreState = self.STATE_DISABLED

        trackColors = (self.trackColor, self.trackColorHighlighted, self.trackColorActivated, self.trackColorDisabled)
        thumbColors = (self.thumbColor, self.thumbColorHighlighted, self.thumbColorActivated, self.thumbColorDisabled)
        thumbCoreColors = (self.thumbCoreColor, self.thumbCoreColorHighlighted, self.thumbCoreColorActivated, self.thumbCoreColorDisabled)

        trackColor = trackColors[trackState]
        thumbColor = thumbColors[thumbState]
        thumbCoreColor = thumbCoreColors[thumbCoreState]


        with renderer.contextForSolidRects(trackColor, blend=blendstate.defaultOpaque) as r2:
            r2.add(trackRect)

        if self.borderWidth > 0:
            with renderer.contextForSolidEllipses(self.borderColor, blend=blendstate.defaultAlpha) as r2:
                r2.add(thumbRect)

            thumbRect.x += self.borderWidth
            thumbRect.y += self.borderWidth
            thumbRect.width -= self.borderWidth * 2
            thumbRect.height -= self.borderWidth * 2

        thumbCoreRect = core.Rect(0,0,self.trackSize, self.trackSize)
        thumbCoreRect.center = thumbRect.center

        with renderer.contextForSolidEllipses(thumbColor, blend=blendstate.defaultAlpha) as r2:
            r2.add(thumbRect)
        with renderer.contextForSolidEllipses(thumbCoreColor, blend=blendstate.defaultAlpha) as r2:
            r2.add(thumbCoreRect)


    def setValue(self, value):
        if value > self.maximumValue: value = self.maximumValue
        if value < self.minimumValue: value = self.minimumValue
        if value != self.__updatedValue:
            self.__updatedValue = value

    def hitTest(self, pos):
        trackRect = self.trackRect()
        if trackRect.isInside(pos):
            return True
        thumbCenter = self.thumbCenter()
        length = (pos.vector() - thumbCenter.vector()).length()
        if length <= self.thumbRadius:
            return True
        return False

    def onMouseDown(self, deviceId, buttonId, pos):
        if self.__capturedMouse is None:
            self.captureMouse(deviceId)
            self.__capturedMouse = deviceId

            thumbCenter = self.thumbCenter()
            length = (pos.vector() - thumbCenter.vector()).length()
            thumbClick = length <= self.thumbRadius
            if thumbClick:
                self.__thumbActive = True
                self._thumbOffset = pos.vector() - thumbCenter.vector()
                self.redraw()
            else:
                trackRect = self.trackRect()
                if trackRect.isInside(pos):
                    if self.vertical:
                        value = (pos.y - self.thumbRadius) / trackRect.height
                    else:
                        value = (pos.x - self.thumbRadius) / trackRect.width

                    range = self.maximumValue - self.minimumValue
                    self.setValue(value * range + self.minimumValue)


    def onMouseUp(self, deviceId, buttonId, pos):
        if self.__capturedMouse == deviceId:
            self.releaseMouse(deviceId)
            self.__capturedMouse = None

            if self.__thumbActive:
                self.__thumbActive = False
                self.redraw()

    def onMouseMove(self, deviceId, pos, delta):
        if self.__capturedMouse == deviceId:
            if self.__thumbActive:
                trackRect = self.trackRect()
                if self.vertical:
                    length = trackRect.height
                    offset = pos.y - self.thumbRadius - self._thumbOffset.y
                else:
                    length = trackRect.width
                    offset = pos.x - self.thumbRadius - self._thumbOffset.x

                if offset < 0.0:
                    value = self.minimumValue
                elif offset > length:
                    value = self.maximumValue
                else:
                    range = self.maximumValue - self.minimumValue
                    value = ((offset / length) * range) + self.minimumValue

                self.setValue(value)

        if deviceId == 0:
            self.updateMouseHover(pos)

    def onMouseLeave(self, deviceId):
        if deviceId == 0:
            if self.__thumbHover or self.__trackHover:
                self.__thumbHover = False
                self.__trackHover = False
                self.redraw()
