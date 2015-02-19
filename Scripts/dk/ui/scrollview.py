import _dk_core as core
from .. import blendstate
from .. import window
from . import view


_POS_VERTICAL_SCROLL_SLIDER = 1
_POS_VERTICAL_SCROLL_TRACK = 2
_POS_HORIZONTAL_SCROLL_SLIDER = 3
_POS_HORIZONTAL_SCROLL_TRACK = 4
_POS_ZOOM_BUTTON = 5

from collections import namedtuple

_MouseTrackPos = namedtuple('_MouseTrackPos', 'mouseId, type, offset')

class ScrollView(view.View):

    scrollBarBorderWidth = 1
    scrollBarSize = 16
    scrollSliderMinimumLength = 10

    showVerticalScrollBar = False
    showHorizontalScrollBar = False
    showZoomButton = False
    leftScrollBar = False

    minimumZoomScale = 0.1
    maximumZoomScale = 10

    _activatedColor = core.Color(0.6, 0.6, 1.0)
    _highlightedColor = core.Color(0.8, 0.8, 1.0)
    _disabledColor = core.Color(0.4, 0.4, 0.4)


    scrollTrackColor = core.Color(0.5, 0.5, 0.5)
    scrollTrackColorHighlighted = core.Color(0.65, 0.65, 0.65)
    scrollTrackColorDisabled = _disabledColor

    scrollSliderColor = core.Color(0.87, 0.87, 0.87)
    scrollSliderColorHighlighted = _highlightedColor
    scrollSliderColorActivated = _activatedColor

    zoomButtonColor = core.Color(0.87, 0.87, 0.87)
    zoomButtonColorHighlighted = _highlightedColor
    zoomButtonColorActivated = _activatedColor
    zoomButtonColorDisabled = _disabledColor

    def __init__(self, contentSize=None, *args, **kwargs):
        super().__init__(*args, **kwargs)

        self.__zoomScale = 1.0
        self.__contentOffset = core.Point(0, 0)
        if contentSize:
            assert isinstance(contentSize, core.Size)
            scaleX, scaleY = self.contentScale
            width = max(contentSize.width, scaleX)
            height = max(contentSize.height, scaleY)
            self.__contentSize = core.Size(width, height)
        else:
            self.__contentSize = core.Size(0, 0)

        self.__verticalScrollTrackRect = None
        self.__verticalScrollSliderRect = None
        self.__horizontalScrollTrackRect = None
        self.__horizontalScrollSliderRect = None
        self.__zoomButtonRect = None

        self.__mouseHoverPos = 0
        self.__mouseTrackInfo = None
        self.__updateContentTransform = False

    @property
    def contentOffset(self):
        return self.__contentOffset

    @contentOffset.setter
    def contentOffset(self, value):
        self.__contentOffset = value
        self.__updateContentTransform = True

    @property
    def contentSize(self):
        return self.__contentSize

    @contentSize.setter
    def contentSize(self, value):
        self.__contentSize = value
        self.__updateContentTransform = True

    @property
    def zoomScale(self):
        return self.__zoomScale

    @zoomScale.setter
    def zoomScale(self, value):
        self.__zoomScale = value
        self.__updateContentTransform = True

    def layout(self):
        if self.layouter:
            self.layouter(self.contentBounds(), self.children())

    def onLoaded(self):
        super().onLoaded()
        self._updateScrollTrackRect()
        self.updateContentTransform()

    def contentBounds(self):
        """
        calculate actual display bounds.
        bounds NOT INCLUDES scroll-bar area!!
        """
        border = round(self.scrollBarBorderWidth)
        bounds = super().contentBounds()

        if bounds.width < border + self.scrollBarSize:
            self.showVerticalScrollBar = False
        if bounds.height < border + self.scrollBarSize:
            self.showHorizontalScrollBar = False

        if self.showVerticalScrollBar:
            bounds.width -= self.scrollBarSize + border
            if self.leftScrollBar:
                bounds.x += self.scrollBarSize + border
        if self.showHorizontalScrollBar:
            bounds.height -= self.scrollBarSize + border
            bounds.y += self.scrollBarSize + border
        return bounds

    def contentDisplayBounds(self):
        """
        calculate content-bounds NOT INCLUDES scroll-bar area.
        result bounds is not same as visible area. (applied with scale, includes scrollable area)
        """
        border = round(self.scrollBarBorderWidth)
        bounds = super().contentBounds()
        bounds = super().unprojectLocalRect(bounds)

        if bounds.width < border + self.scrollBarSize:
            self.showVerticalScrollBar = False
        if bounds.height < border + self.scrollBarSize:
            self.showHorizontalScrollBar = False

        if self.showVerticalScrollBar:
            bounds.width -= self.scrollBarSize + border
            if self.leftScrollBar:
                bounds.x += self.scrollBarSize + border
        if self.showHorizontalScrollBar:
            bounds.height -= self.scrollBarSize + border
            bounds.y += self.scrollBarSize + border
        return bounds


    def maxScrollOffset(self):
        bounds = self.contentBounds()
        scaleX = bounds.width / self.__zoomScale
        scaleY = bounds.height / self.__zoomScale

        maxX = max(self.__contentSize.width - scaleX, 0)
        maxY = max(self.__contentSize.height - scaleY, 0)

        return maxX, maxY

    def _updateScrollSliderRect(self):

        self.__verticalScrollSliderRect = None
        self.__horizontalScrollSliderRect = None

        if self.showVerticalScrollBar or self.showHorizontalScrollBar:
            # print('_updateScrollSliderRect ({})'.format(core.Timer.tick()))

            bounds = self.contentBounds()

            if self.showVerticalScrollBar:
                scale = bounds.height / self.__zoomScale
                scrollMax = self.__contentSize.height - scale
                if scrollMax > 0 and self.__verticalScrollTrackRect.height > 2:
                    rc = core.Rect(self.__verticalScrollTrackRect)
                    pos = self.__contentOffset.y / scrollMax
                    height = rc.height

                    sliderMinLength = self.scrollSliderMinimumLength
                    if sliderMinLength >= rc.height:
                        sliderMinLength = rc.height * 0.5

                    rc.height = (rc.height - sliderMinLength) * scale / self.__contentSize.height + sliderMinLength
                    rc.y += (height - rc.height) * pos
                    self.__verticalScrollSliderRect = rc

            if self.showHorizontalScrollBar:
                scale = bounds.width / self.__zoomScale
                scrollMax = self.__contentSize.width - scale
                if scrollMax > 0 and self.__horizontalScrollTrackRect.height > 2:
                    rc = core.Rect(self.__horizontalScrollTrackRect)
                    pos = self.__contentOffset.x / scrollMax
                    width = rc.width

                    sliderMinLength = self.scrollSliderMinimumLength
                    if sliderMinLength >= rc.width:
                        sliderMinLength = rc.width * 0.5

                    rc.width = (rc.width - sliderMinLength) * scale / self.__contentSize.width + sliderMinLength
                    rc.x += (width - rc.width) * pos
                    self.__horizontalScrollSliderRect = rc


    def _updateScrollTrackRect(self):
        self.__verticalScrollTrackRect = None
        self.__horizontalScrollTrackRect = None
        self.__zoomButtonRect = None

        if self.showVerticalScrollBar or self.showHorizontalScrollBar:
            # print('_updateScrollTrackRect')

            border = round(self.scrollBarBorderWidth)
            bounds = self.contentBounds()
            scrollBarSize = self.scrollBarSize

            if self.showVerticalScrollBar:
                rc = core.Rect()
                rc.width = scrollBarSize
                if self.leftScrollBar:
                    rc.x = bounds.x - scrollBarSize - border
                else:
                    rc.x = bounds.x + bounds.width + border
                if not self.showHorizontalScrollBar and self.showZoomButton:
                    rc.y = bounds.y + scrollBarSize + border
                    rc.height = bounds.height - scrollBarSize
                else:
                    rc.y = bounds.y
                    rc.height = bounds.height
                self.__verticalScrollTrackRect = rc

            if self.showHorizontalScrollBar:
                rc = core.Rect()
                rc.x = bounds.x
                rc.y = bounds.y - scrollBarSize - border
                rc.height = scrollBarSize
                if not self.showVerticalScrollBar and self.showZoomButton:
                    rc.width = bounds.width - scrollBarSize + border
                    if self.leftScrollBar:
                        rc.x = bounds.x + scrollBarSize + border
                else:
                    rc.width = bounds.width
                self.__horizontalScrollTrackRect = rc

            zoomRectShouldVisible = self.showVerticalScrollBar and self.showHorizontalScrollBar
            if zoomRectShouldVisible or self.showZoomButton:
                rc = core.Rect()
                rc.width = scrollBarSize
                rc.height = scrollBarSize
                if self.showVerticalScrollBar:
                    if self.leftScrollBar:
                        rc.x = bounds.x - scrollBarSize - border
                    else:
                        rc.x = bounds.x + bounds.width + border
                else:
                    if self.leftScrollBar:
                        rc.x = bounds.x
                    else:
                        rc.x = bounds.x + bounds.width - scrollBarSize
                if self.showHorizontalScrollBar:
                    rc.y = bounds.y - scrollBarSize - border
                else:
                    rc.y = bounds.y
                self.__zoomButtonRect = rc

            self._updateScrollSliderRect()

    def onResized(self):
        super().onResized()
        self.updateContentTransform()
        if self.surfaceVisibilityTest:
            if self.showHorizontalScrollBar or self.showVerticalScrollBar:
                self.surfaceVisibilityTest = False

        self._updateScrollTrackRect()

    def onUpdate(self, delta, tick, date):
        super().onUpdate(delta, tick, date)
        if self.__updateContentTransform:
            self.updateContentTransform()
        
    def onRender(self, renderer):
        super().onRender(renderer)

        border = round(self.scrollBarBorderWidth) / self.__zoomScale
        border2 = border * 2

        trackPos = self.__mouseTrackInfo.type if self.__mouseTrackInfo else 0
        hoverPos = self.__mouseHoverPos

        if self.__verticalScrollTrackRect:
            rc = self.unprojectLocalRect(self.__verticalScrollTrackRect)

            if border > 0:
                with renderer.contextForSolidRects(self.borderColor, blend=blendstate.defaultOpaque) as r:
                    r.add(core.Rect(rc.x - border, rc.y - border, rc.width + border2, rc.height + border2))

            color = self.scrollTrackColorHighlighted if hoverPos == _POS_VERTICAL_SCROLL_TRACK \
                else self.scrollTrackColor if self.__verticalScrollSliderRect \
                else self.scrollTrackColorDisabled

            with renderer.contextForSolidRects(color, blend=blendstate.defaultOpaque) as r:
                r.add(rc)

            if self.__verticalScrollSliderRect:
                rc = self.unprojectLocalRect(self.__verticalScrollSliderRect)

                color = self.scrollSliderColorActivated if trackPos == _POS_VERTICAL_SCROLL_SLIDER \
                    else self.scrollSliderColorHighlighted if hoverPos == _POS_VERTICAL_SCROLL_SLIDER \
                    else self.scrollSliderColor

                with renderer.contextForSolidRects(color, blend=blendstate.defaultOpaque) as r:
                    r.add(rc)

        if self.__horizontalScrollTrackRect:
            rc = self.unprojectLocalRect(self.__horizontalScrollTrackRect)
            if border > 0:
                with renderer.contextForSolidRects(self.borderColor, blend=blendstate.defaultOpaque) as r:
                    r.add(core.Rect(rc.x - border, rc.y - border, rc.width + border2, rc.height + border2))

            color = self.scrollTrackColorHighlighted if hoverPos == _POS_HORIZONTAL_SCROLL_TRACK \
                else self.scrollTrackColor if self.__horizontalScrollSliderRect \
                else self.scrollTrackColorDisabled

            with renderer.contextForSolidRects(color, blend=blendstate.defaultOpaque) as r:
                r.add(rc)

            if self.__horizontalScrollSliderRect:
                rc = self.unprojectLocalRect(self.__horizontalScrollSliderRect)

                color = self.scrollSliderColorActivated if trackPos == _POS_HORIZONTAL_SCROLL_SLIDER \
                    else self.scrollSliderColorHighlighted if hoverPos == _POS_HORIZONTAL_SCROLL_SLIDER \
                    else self.scrollSliderColor

                with renderer.contextForSolidRects(color, blend=blendstate.defaultOpaque) as r:
                    r.add(rc)

        if self.__zoomButtonRect:
            rc = self.unprojectLocalRect(self.__zoomButtonRect)
            if border > 0:
                with renderer.contextForSolidRects(self.borderColor, blend=blendstate.defaultOpaque) as r:
                    r.add(core.Rect(rc.x - border, rc.y - border, rc.width + border2, rc.height + border2))

            activated = trackPos == _POS_ZOOM_BUTTON
            if activated:
                pos = self.__mouseTrackInfo.offset + self.__zoomButtonRect.origin
                activated = self.__zoomButtonRect.isInside(pos)

            color = self.zoomButtonColorDisabled if not self.showZoomButton \
                else self.zoomButtonColorActivated if activated \
                else self.zoomButtonColorHighlighted if hoverPos == _POS_ZOOM_BUTTON \
                else self.zoomButtonColor

            with renderer.contextForSolidRects(color, blend=blendstate.defaultOpaque) as r:
                r.add(rc)

        x, y = self.contentScale
        w, h = self.contentResolution
        scaleX = w / x
        scaleY = h / y

        bounds = self.contentBounds()
        renderer.viewport = bounds.x * scaleX, bounds.y * scaleY, bounds.width * scaleX, bounds.height * scaleY
        renderer.bounds = bounds


    def updateContentTransform(self):
        clamp = lambda x, minV, maxV: minV if x < minV else maxV if x > maxV else x

        if self.minimumZoomScale < 0.1:
            self.minimumZoomScale = 0.1
        if self.maximumZoomScale < self.minimumZoomScale:
            self.maximumZoomScale = self.minimumZoomScale

        self.__zoomScale = clamp(self.__zoomScale, self.minimumZoomScale, self.maximumZoomScale)

        bounds = self.contentBounds()
        scaleX = bounds.width / self.__zoomScale
        scaleY = bounds.height / self.__zoomScale

        maxX = max(self.__contentSize.width - scaleX, 0)
        maxY = max(self.__contentSize.height - scaleY, 0)

        self.__contentOffset.x = clamp(self.__contentOffset.x, 0, maxX)
        self.__contentOffset.y = clamp(self.__contentOffset.y, 0, maxY)

        scale = core.LinearTransform2()
        scale.scale(self.__zoomScale)
        transform = core.AffineTransform2()
        transform.translate(-self.__contentOffset.x, -self.__contentOffset.y)
        transform.multiply(scale)
        transform.translate(bounds.x, bounds.y)
        tm = transform.matrix3()
        if self.contentTransform != tm:
            self.contentTransform = tm
            self._updateScrollSliderRect()
            self.redraw()
        self.__updateContentTransform = False

    def onClickZoomButton(self):
        print('user click zoom-button')
        if self.__zoomScale != 1.0:
            self.__zoomScale = 1.0
        else:
            self.__zoomScale = self.minimumZoomScale
        self.updateContentTransform()

    def setContentOffset(self, x, y):
        if self.__contentOffset.x != x or self.__contentOffset.y != y:
            self.__contentOffset.tuple = x, y
            self.updateContentTransform()

    def onMouseDown(self, deviceId, buttonId, pos):
        super().onMouseDown(deviceId, buttonId, pos)
        if not self.__mouseTrackInfo and not self.isMouseCapturedBySelf(deviceId):
            bounds = self.contentBounds()
            pos = self.projectLocalPoint(pos)
            if not bounds.isInside(pos):
                mouseId = (deviceId, buttonId)
                if self.__verticalScrollSliderRect:
                    if self.__verticalScrollSliderRect.isInside(pos):
                        self.captureMouse(deviceId)
                        offset = pos - self.__verticalScrollSliderRect.origin
                        self.__mouseTrackInfo = _MouseTrackPos(mouseId, _POS_VERTICAL_SCROLL_SLIDER, offset)
                        if self.__mouseHoverPos in (_POS_VERTICAL_SCROLL_SLIDER, _POS_VERTICAL_SCROLL_TRACK):
                            self.__mouseHoverPos = 0     # release mouse hover!
                        self.redraw()
                        return
                    if self.__verticalScrollTrackRect.isInside(pos):
                        range = self.__verticalScrollTrackRect.height - self.__verticalScrollSliderRect.height
                        if range > 0:
                            offset = pos.y - self.__verticalScrollTrackRect.y - self.__verticalScrollSliderRect.height * 0.5
                            maxValue = max(self.__contentSize.height - bounds.height / self.__zoomScale, 0)
                            offset = 0.0 if offset < 0.0 else maxValue if offset > range else offset * maxValue / range
                            self.screen().postOperation(self.setContentOffset, (self.__contentOffset.x, offset))
                        return

                if self.__horizontalScrollSliderRect:
                    if self.__horizontalScrollSliderRect.isInside(pos):
                        self.captureMouse(deviceId)
                        offset = pos - self.__horizontalScrollSliderRect.origin
                        self.__mouseTrackInfo = _MouseTrackPos(mouseId, _POS_HORIZONTAL_SCROLL_SLIDER, offset)
                        if self.__mouseHoverPos in (_POS_HORIZONTAL_SCROLL_SLIDER, _POS_HORIZONTAL_SCROLL_TRACK):
                            self.__mouseHoverPos = 0     # release mouse hover
                        self.redraw()
                        return
                    if self.__horizontalScrollTrackRect.isInside(pos):
                        range = self.__horizontalScrollTrackRect.width - self.__horizontalScrollSliderRect.width
                        if range > 0:
                            offset = pos.x - self.__horizontalScrollTrackRect.x - self.__horizontalScrollSliderRect.width * 0.5
                            maxValue = max(self.__contentSize.width - bounds.width / self.__zoomScale, 0)
                            offset = 0.0 if offset < 0.0 else maxValue if offset > range else offset * maxValue / range
                            self.screen().postOperation(self.setContentOffset, (offset, self.__contentOffset.y))
                        return

                if self.showZoomButton and self.__zoomButtonRect:
                    if self.__zoomButtonRect.isInside(pos):
                        self.captureMouse(deviceId)
                        offset = pos - self.__zoomButtonRect.origin
                        self.__mouseTrackInfo = _MouseTrackPos(mouseId, _POS_ZOOM_BUTTON, offset)
                        if self.__mouseHoverPos == _POS_ZOOM_BUTTON:
                            self.__mouseHoverPos = 0
                        self.redraw()
                        return

    def onMouseMove(self, deviceId, pos, delta):
        super().onMouseMove(deviceId, pos, delta)
        if self.__mouseTrackInfo and self.__mouseTrackInfo.mouseId[0] == deviceId:
            if self.isMouseCapturedBySelf(deviceId):
                bounds = self.contentBounds()
                pos = self.projectLocalPoint(pos)
                if self.__mouseTrackInfo.type == _POS_ZOOM_BUTTON:
                    pos_old = core.Point(self.__mouseTrackInfo.offset + self.__zoomButtonRect.origin)
                    act1 = self.__zoomButtonRect.isInside(pos_old)
                    act2 = self.__zoomButtonRect.isInside(pos)
                    self.__mouseTrackInfo.offset.tuple = (pos - self.__zoomButtonRect.origin).tuple
                    if act1 != act2:        # zoom-button activation state changed.
                        self.redraw()
                elif self.__mouseTrackInfo.type == _POS_VERTICAL_SCROLL_SLIDER:
                    if self.__verticalScrollSliderRect:
                        range = self.__verticalScrollTrackRect.height - self.__verticalScrollSliderRect.height
                        if range > 0:
                            offset = pos.y - self.__mouseTrackInfo.offset.y - self.__verticalScrollTrackRect.y
                            maxValue = max(self.__contentSize.height - bounds.height / self.__zoomScale, 0)
                            offset = 0.0 if offset < 0.0 else maxValue if offset > range else offset * maxValue / range
                            self.screen().postOperation(self.setContentOffset, (self.__contentOffset.x, offset))
                elif self.__mouseTrackInfo.type == _POS_HORIZONTAL_SCROLL_SLIDER:
                    if self.__horizontalScrollSliderRect:
                        range = self.__horizontalScrollTrackRect.width - self.__horizontalScrollSliderRect.width
                        if range > 0:
                            offset = pos.x - self.__mouseTrackInfo.offset.x - self.__horizontalScrollTrackRect.x
                            maxValue = max(self.__contentSize.width - bounds.width / self.__zoomScale, 0)
                            offset = 0.0 if offset < 0.0 else maxValue if offset > range else offset * maxValue / range
                            self.screen().postOperation(self.setContentOffset, (offset, self.__contentOffset.y))
            else:
                self.__mouseTrackInfo = None
                self.redraw()
        elif deviceId == 0:
            hoverPos = 0
            if not self.isMouseCapturedBySelf(deviceId):
                bounds = self.contentBounds()
                pos = self.projectLocalPoint(pos)
                if not bounds.isInside(pos):
                    trackPos = self.__mouseTrackInfo.type if self.__mouseTrackInfo else 0
                    if hoverPos == 0 and trackPos not in (_POS_VERTICAL_SCROLL_SLIDER, _POS_VERTICAL_SCROLL_TRACK):
                        # check mouse hover on vertical scroll
                        if self.__verticalScrollSliderRect:
                            if self.__verticalScrollSliderRect.isInside(pos):
                                hoverPos = _POS_VERTICAL_SCROLL_SLIDER
                            elif self.__verticalScrollTrackRect.isInside(pos):
                                hoverPos = _POS_VERTICAL_SCROLL_TRACK
                    if hoverPos == 0 and trackPos not in (_POS_HORIZONTAL_SCROLL_SLIDER, _POS_HORIZONTAL_SCROLL_TRACK):
                        # check mouse hover on horizontal scroll
                        if self.__horizontalScrollSliderRect:
                            if self.__horizontalScrollSliderRect.isInside(pos):
                                hoverPos = _POS_HORIZONTAL_SCROLL_SLIDER
                            elif self.__horizontalScrollTrackRect.isInside(pos):
                                hoverPos = _POS_HORIZONTAL_SCROLL_TRACK
                    if hoverPos == 0 and trackPos != _POS_ZOOM_BUTTON:
                        if self.showZoomButton and self.__zoomButtonRect:
                            if self.__zoomButtonRect.isInside(pos):
                                hoverPos = _POS_ZOOM_BUTTON

            if self.__mouseHoverPos != hoverPos:
                self.__mouseHoverPos = hoverPos
                self.redraw()


    def onMouseUp(self, deviceId, buttonId, pos):
        super().onMouseUp(deviceId, buttonId, pos)
        if self.__mouseTrackInfo and self.__mouseTrackInfo.mouseId == (deviceId, buttonId):
            bounds = self.contentBounds()
            pos = self.projectLocalPoint(pos)

            if self.__mouseTrackInfo.type == _POS_ZOOM_BUTTON:
                if self.__zoomButtonRect and self.__zoomButtonRect.isInside(pos):
                    self.screen().postOperation(self.onClickZoomButton, ())

            if deviceId == 0 and self.__mouseHoverPos == 0:
                self.__mouseHoverPos = self.__mouseTrackInfo.type

            self.releaseMouse(deviceId)
            self.__mouseTrackInfo = None
            self.redraw()

    def onMouseLeave(self, deviceId):
        super().onMouseLeave(deviceId)
        if deviceId == 0 and self.__mouseHoverPos:
            self.__mouseHoverPos = 0
            if not self.__mouseTrackInfo:
                self.redraw()

    def onMouseWheel(self, deviceId, pos, delta):
        super().onMouseWheel(deviceId, pos, delta)
        window = self.screen().window
        from .. import vkey
        if window.isKeyDown(0, vkey.LEFT_OPTION):
            offset = 0.05 if delta.y > 0.0 else -0.05
            zoomScale = self.__zoomScale + offset
            self.__zoomScale = max(zoomScale, 0.1)
        else:
            # OS X does handles vertical scroll while holding down shift-key.
            # if window.isKeyDown(0, vkey.LEFT_SHIFT):
            #     self.__contentOffset.x -= delta.y * 10
            #     self.__contentOffset.y += delta.x * 10
            # else:
            self.__contentOffset.x -= delta.x * 10
            self.__contentOffset.y += delta.y * 10
        self.updateContentTransform()

    def preprocessMouseEvent(self, type, deviceId, buttonId, pos, delta):
        if type == window.MOUSE_EVENT_MOVE and self.__mouseHoverPos:
            if self.screen().focusFrame(deviceId) is not None:

                bounds = self.contentBounds()
                pos = self.projectLocalPoint(pos)
                if bounds.isInside(pos):
                    self.__mouseHoverPos = 0
                    self.redraw()
            else:
                self.__mouseHoverPos = 0
                self.redraw()

        # 마우스가 자식 위로 올라가면.. 호버 없애야 함.
        return super().preprocessMouseEvent(type, deviceId, buttonId, pos, delta)
